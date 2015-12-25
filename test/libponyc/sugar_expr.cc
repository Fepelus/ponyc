#include <gtest/gtest.h>
#include <platform.h>

#include "util.h"


// Tests for sugar that is applied during the expr pass

#define TEST_COMPILE(src) DO(test_compile(src, "expr"))
#define TEST_ERROR(src) DO(test_error(src, "expr"))
#define TEST_EQUIV(src, expect) DO(test_equiv(src, "expr", expect, "expr"))


class SugarExprTest : public PassTest
{};


// Partial application

TEST_F(SugarExprTest, PartialMinimal)
{
  const char* short_form =
    "trait T\n"
    "  fun f()\n"

    "class Foo\n"
    "  fun f(x: T) =>\n"
    "    x~f()";

  const char* full_form =
    "trait T\n"
    "  fun f()\n"

    "class Foo\n"
    "  fun f(x: T) =>\n"
    "    lambda box()(hygid = x) => hygid.f() end";

  TEST_EQUIV(short_form, full_form);
}


TEST_F(SugarExprTest, PartialReceiverCapability)
{
  const char* short_form =
    "trait T\n"
    "  fun ref f()\n"

    "class Foo\n"
    "  fun f(x: T ref) =>\n"
    "    x~f()";

  const char* full_form =
    "trait T\n"
    "  fun ref f()\n"

    "class Foo\n"
    "  fun f(x: T ref) =>\n"
    "    lambda ref()(hygid = x) => hygid.f() end";

  TEST_EQUIV(short_form, full_form);
}


TEST_F(SugarExprTest, PartialReceiverCapabilityFail)
{
  const char* short_form =
    "trait T\n"
    "  fun ref f()\n"

    "class Foo\n"
    "  fun f(x: T box) =>\n"
    "    x~f()";

  TEST_ERROR(short_form);
}


TEST_F(SugarExprTest, PartialMemberAccessFail)
{
  const char* short_form =
    "class Foo\n"
    "  let y: U32 = 4"
    "  fun f() =>\n"
    "    this~y";

  TEST_ERROR(short_form);
}


TEST_F(SugarExprTest, PartialTupleElementAccessFail)
{
  const char* short_form =
    "trait T\n"
    "  fun f()\n"

    "class Foo\n"
    "  fun f(x: (T, T)) =>\n"
    "    x~_1()";

  TEST_ERROR(short_form);
}


TEST_F(SugarExprTest, PartialSomeArgs)
{
  const char* short_form =
    "trait T\n"
    "  fun f(a: U32, b: U32, c: U32, d: U32)\n"

    "class Foo\n"
    "  fun f(x: T) =>\n"
    "    x~f(3 where c = 4)";

  const char* full_form =
    "trait T\n"
    "  fun f(a: U32, b: U32, c: U32, d: U32)\n"

    "class Foo\n"
    "  fun f(x: T) =>\n"
    "    lambda box(b: U32, d: U32)\n"
    "      (hygid = x, a: U32 = (3), c: U32 = (4)) =>\n"
    "      hygid.f(a, consume b, c, consume d)\n"
    "    end";

  TEST_EQUIV(short_form, full_form);
}


TEST_F(SugarExprTest, PartialAllArgs)
{
  const char* short_form =
    "trait T\n"
    "  fun f(a: U32, b: U32, c: U32, d: U32)\n"

    "class Foo\n"
    "  fun f(x: T) =>\n"
    "    x~f(1, 2, 3, 4)";

  const char* full_form =
    "trait T\n"
    "  fun f(a: U32, b: U32, c: U32, d: U32)\n"

    "class Foo\n"
    "  fun f(x: T) =>\n"
    "    lambda box()(hygid = x, a: U32 = (1), b: U32 = (2), c: U32 = (3),\n"
    "      d: U32 = (4)) =>\n"
    "      hygid.f(a, b, c, d)\n"
    "    end";

  TEST_EQUIV(short_form, full_form);
}


TEST_F(SugarExprTest, PartialDefaultArgs)
{
  const char* short_form =
    "trait T\n"
    "  fun f(a: U32 = 1, b: U32 = 2, c: U32 = 3, d: U32 = 4)\n"

    "class Foo\n"
    "  fun f(x: T) =>\n"
    "    x~f(5 where c = 6)";

  const char* full_form =
    "trait T\n"
    "  fun f(a: U32 = 1, b: U32 = 2, c: U32 = 3, d: U32 = 4)\n"

    "class Foo\n"
    "  fun f(x: T) =>\n"
    "    lambda box(b: U32 = 2, d: U32 = 4)\n"
    "      (hygid = x, a: U32 = (5), c: U32 = (6)) =>\n"
    "      hygid.f(a, consume b, c, consume d)\n"
    "    end";

  TEST_EQUIV(short_form, full_form);
}


TEST_F(SugarExprTest, PartialResult)
{
  const char* short_form =
    "trait T\n"
    "  fun f(): U32\n"

    "class Foo\n"
    "  fun f(x: T) =>\n"
    "    x~f()";

  const char* full_form =
    "trait T\n"
    "  fun f(): U32\n"

    "class Foo\n"
    "  fun f(x: T) =>\n"
    "    lambda box()(hygid = x): U32 => hygid.f() end";

  TEST_EQUIV(short_form, full_form);
}


TEST_F(SugarExprTest, PartialBehaviour)
{
  const char* short_form =
    "trait T\n"
    "  be f()\n"

    "class Foo\n"
    "  fun f(x: T) =>\n"
    "    x~f()";

  const char* full_form =
    "trait T\n"
    "  be f()\n"

    "class Foo\n"
    "  fun f(x: T) =>\n"
    "    lambda box()(hygid = x): T tag => hygid.f() end";

  TEST_EQUIV(short_form, full_form);
}


TEST_F(SugarExprTest, PartialRaisesError)
{
  const char* short_form =
    "trait T\n"
    "  fun f(): U32 ?\n"

    "class Foo\n"
    "  fun f(x: T) =>\n"
    "    x~f()";

  const char* full_form =
    "trait T\n"
    "  fun f(): U32 ?\n"

    "class Foo\n"
    "  fun f(x: T) =>\n"
    "    lambda box()(hygid = x): U32 ? => hygid.f() end";

  TEST_EQUIV(short_form, full_form);
}


TEST_F(SugarExprTest, PartialParameterTypeParameters)
{
  const char* short_form =
    "trait T\n"
    "  fun f(a: T box)\n"

    "class Foo[A: T #read]\n"
    "  fun f(t: T box, a: A) =>\n"
    "    t~f(a)";

  const char* full_form =
    "trait T\n"
    "  fun f(a: T box)\n"

    "class Foo[A: T #read]\n"
    "  fun f(t: T box, a: A) =>\n"
    "    lambda box()(hygid = t, a: A = (a)) => hygid.f(a) end";

  TEST_EQUIV(short_form, full_form);
}


TEST_F(SugarExprTest, PartialReceiverTypeParameters)
{
  const char* short_form =
    "trait T\n"
    "  fun f()\n"

    "class Foo[A: T #read]\n"
    "  fun f(x: A) =>\n"
    "    x~f()";

  const char* full_form =
    "trait T\n"
    "  fun f()\n"

    "class Foo[A: T #read]\n"
    "  fun f(x: A) =>\n"
    "    lambda box()(hygid = x) => hygid.f() end";

  TEST_EQUIV(short_form, full_form);
}


TEST_F(SugarExprTest, PartialFunctionTypeParameter)
{
  const char* short_form =
    "trait T\n"
    "  fun f[A](a: A)\n"

    "class Foo\n"
    "  fun f(x: T) =>\n"
    "    x~f[U32](3)";

  const char* full_form =
    "trait T\n"
    "  fun f[A](a: A)\n"

    "class Foo\n"
    "  fun f(x: T) =>\n"
    "    lambda box()(hygid = x, a: U32 = (3)) => hygid.f[U32](a) end";

  TEST_EQUIV(short_form, full_form);
}


// Lambdas

TEST_F(SugarExprTest, LambdaMinimal)
{
  const char* short_form =
    "class Foo\n"
    "  fun f() =>\n"
    "    lambda() => None end";

  const char* full_form =
    "class Foo\n"
    "  fun f() =>\n"
    "    object\n"
    "      fun apply() => None\n"
    "    end";

  TEST_EQUIV(short_form, full_form);
}


TEST_F(SugarExprTest, LambdaFull)
{
  const char* short_form =
    "trait A\n"
    "trait B\n"
    "trait C\n"
    "trait D\n"
    "trait D2 is D\n"

    "class Foo\n"
    "  fun f(c: C val, d: D2 val) =>\n"
    "    lambda iso (a: A, b: B)(c, _c = c, _d: D val = d): A => a end";

  const char* full_form =
    "trait A\n"
    "trait B\n"
    "trait C\n"
    "trait D\n"
    "trait D2 is D\n"

    "class Foo\n"
    "  fun f(c: C val, d: D2 val) =>\n"
    "    object iso\n"
    "      let c: C val = c\n"
    "      let _c: C val = c\n"
    "      let _d: D val = d\n"
    "      fun iso apply(a: A, b: B): A => a\n"
    "    end";

  TEST_EQUIV(short_form, full_form);
}


TEST_F(SugarExprTest, LambdaThrow)
{
  const char* short_form =
    "class Foo\n"
    "  fun f() =>\n"
    "    lambda() ? => error end";

  const char* full_form =
    "class Foo\n"
    "  fun f() =>\n"
    "    object\n"
    "      fun apply() ? => error\n"
    "    end";

  TEST_EQUIV(short_form, full_form);
}


TEST_F(SugarExprTest, LambdaWithTypeArgs)
{
  const char* short_form =
    "trait T\n"

    "class Foo\n"
    "  fun f() =>\n"
    "    lambda[A: T]() => None end";

  const char* full_form =
    "trait T\n"

    "class Foo\n"
    "  fun f() =>\n"
    "    object\n"
    "      fun apply[A: T]() => None\n"
    "    end";

  TEST_EQUIV(short_form, full_form);
}


TEST_F(SugarExprTest, LambdaCaptureLocalLet)
{
  const char* short_form =
    "trait T\n"

    "class Foo\n"
    "  fun f() =>\n"
    "    let x: U32 = 4\n"
    "    lambda()(x) => None end";

  const char* full_form =
    "trait T\n"

    "class Foo\n"
    "  fun f() =>\n"
    "    let x: U32 = 4\n"
    "    object\n"
    "      let x: U32 = x"
    "      fun apply() => None\n"
    "    end";

  TEST_EQUIV(short_form, full_form);
}


TEST_F(SugarExprTest, LambdaCaptureLocalVar)
{
  const char* short_form =
    "trait T\n"

    "class Foo\n"
    "  fun f() =>\n"
    "    var x: U32 = 4\n"
    "    lambda()(x) => None end";

  const char* full_form =
    "trait T\n"

    "class Foo\n"
    "  fun f() =>\n"
    "    var x: U32 = 4\n"
    "    object\n"
    "      let x: U32 = x"
    "      fun apply() => None\n"
    "    end";

  TEST_EQUIV(short_form, full_form);
}


TEST_F(SugarExprTest, LambdaCaptureParameter)
{
  const char* short_form =
    "trait T\n"

    "class Foo\n"
    "  fun f(x: U32) =>\n"
    "    lambda()(x) => None end";

  const char* full_form =
    "trait T\n"

    "class Foo\n"
    "  fun f(x: U32) =>\n"
    "    object\n"
    "      let x: U32 = x"
    "      fun apply() => None\n"
    "    end";

  TEST_EQUIV(short_form, full_form);
}


TEST_F(SugarExprTest, LambdaCaptureFieldLet)
{
  const char* short_form =
    "trait T\n"

    "class Foo\n"
    "  let x: U32 = 4\n"
    "  fun f() =>\n"
    "    lambda()(x) => None end";

  const char* full_form =
    "trait T\n"

    "class Foo\n"
    "  let x: U32 = 4\n"
    "  fun f() =>\n"
    "    object\n"
    "      let x: U32 = x"
    "      fun apply() => None\n"
    "    end";

  TEST_EQUIV(short_form, full_form);
}


TEST_F(SugarExprTest, LambdaCaptureFieldVar)
{
  const char* short_form =
    "trait T\n"

    "class Foo\n"
    "  var x: U32 = 4\n"
    "  fun f() =>\n"
    "    lambda()(x) => None end";

  const char* full_form =
    "trait T\n"

    "class Foo\n"
    "  var x: U32 = 4\n"
    "  fun f() =>\n"
    "    object\n"
    "      let x: U32 = x"
    "      fun apply() => None\n"
    "    end";

  TEST_EQUIV(short_form, full_form);
}


// Early sugar that may cause errors in type checking

TEST_F(SugarExprTest, ObjectLiteralReferencingTypeParameters)
{
  const char* short_form =
    "trait T\n"

    "class Foo[A: T]\n"
    "  fun f(x: A) =>\n"
    "    object let _x: A = consume x end";

  const char* full_form =
    "trait T\n"

    "class Foo[A: T]\n"
    "  fun f(x: A) =>\n"
    "    Hygid[A].create(consume x)\n"

    "class Hygid[A: T]\n"
    "  let _x: A\n"
    "  new create(hygid: A) =>\n"
    "    _x = consume hygid";

  TEST_EQUIV(short_form, full_form);
}
