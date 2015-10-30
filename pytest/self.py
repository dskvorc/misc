val = 1

class SelfTest(object):
    val = 2

    def a(self, arg):
        return self.val + arg

    def b(self, arg):
        return val + arg

print(val)
obj = SelfTest()
print(obj.a(5))
print(obj.b(5))

