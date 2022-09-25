from decimal import Decimal
from time import perf_counter
import ctypes
import decimal
import random
import unittest


class ABIException(Exception):
    pass


class BigInteger(ctypes.Structure):
    def __init__(self, address):
        super(BigInteger, self).__init__()
        self.address = address

    @staticmethod
    def new(argument):
        if address := lib.create_biginteger(ctypes.c_char_p(argument.encode())):
            return BigInteger(address)
        raise ABIException("Maybe number parse error")

    def __mul__(self, other):
        return BigInteger(lib.integer_multiplication(self.address, other.address))

    def __repr__(self):
        return lib.integer_string(self.address).decode()

    def __del__(self):
        lib.delete_integer(self.address)


class BigDecimal(ctypes.Structure):
    def __init__(self, address):
        super(BigDecimal, self).__init__()
        self.address = address

    @staticmethod
    def new(argument):
        if address := lib.create_bigdecimal(ctypes.c_char_p(argument.encode())):
            return BigDecimal(address)
        raise ABIException("Maybe number parse error")

    def __mul__(self, other):
        return BigDecimal(lib.decimal_multiplication(self.address, other.address))

    def __repr__(self):
        return lib.decimal_string(self.address).decode()

    def __del__(self):
        lib.delete_decimal(self.address)


class CorrectnessTest(unittest.TestCase):
    def test_stress_test_integer(self):
        def gen_integer(n):
            return ''.join([str(random.randint(1, 9))] + [str(random.randint(0, 9)) for _ in range(n - 1)])

        def format_no_exponent(d):
            return '{:f}'.format(d.quantize(Decimal(1)) if d == d.to_integral() else d.normalize())

        for i in range(10 * 6):
            max_length = 10 ** (i // 10 + 1)
            length = random.randint(max_length // 10, max_length)
            lhs = gen_integer(length)
            rhs = gen_integer(length)
            integer_lhs = BigInteger.new(lhs)
            integer_rhs = BigInteger.new(rhs)

            start = perf_counter()
            integer_result = integer_lhs * integer_rhs
            duration = perf_counter() - start

            std_result = Decimal(lhs) * Decimal(rhs)

            self.assertEqual(repr(integer_lhs).lstrip('0'), lhs)
            self.assertEqual(repr(integer_rhs).lstrip('0'), rhs)
            self.assertEqual(repr(integer_result).lstrip('0'), format_no_exponent(std_result))

            print(f'BigInteger, Length {length}, time = {duration * 1000:.2f} ms, Correct')

    def test_stress_test_decimal(self):
        def gen_decimal(n):
            integer = ''.join([str(random.randint(0, 9)) for _ in range(n)])
            exponent = random.randint(- n * 2, n)
            return Decimal(integer) * Decimal((0, [1], exponent))

        def format_no_exponent(d):
            return '{:f}'.format(d.quantize(Decimal(1)) if d == d.to_integral() else d.normalize())

        decimal.setcontext(decimal.Context(prec=decimal.MAX_PREC, Emax=decimal.MAX_EMAX, Emin=decimal.MIN_EMIN))

        for i in range(10 * 6):
            max_length = 10 ** (i // 10 + 1)
            length = random.randint(max_length // 10, max_length)
            lhs = gen_decimal(length)
            rhs = gen_decimal(length)
            decimal_lhs = BigDecimal.new(str(lhs))
            decimal_rhs = BigDecimal.new(str(rhs))

            start = perf_counter()
            decimal_result = decimal_lhs * decimal_rhs
            duration = perf_counter() - start

            self.assertEqual(repr(decimal_lhs), format_no_exponent(lhs))
            self.assertEqual(repr(decimal_rhs), format_no_exponent(rhs))
            self.assertEqual(repr(decimal_result), format_no_exponent(lhs * rhs))

            print(f'BigDecimal, Length {length}, time = {duration * 1000:.2f} ms, Correct')


if __name__ == '__main__':
    lib = ctypes.CDLL('libmul_abi@CMAKE_SHARED_LIBRARY_SUFFIX@')
    lib.create_biginteger.restype = ctypes.POINTER(BigInteger)
    lib.integer_multiplication.restype = ctypes.POINTER(BigInteger)
    lib.integer_string.restype = ctypes.c_char_p
    lib.create_bigdecimal.restype = ctypes.POINTER(BigDecimal)
    lib.decimal_multiplication.restype = ctypes.POINTER(BigDecimal)
    lib.decimal_string.restype = ctypes.c_char_p

    unittest.main()
