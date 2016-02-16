﻿#region Copyright
/*
This file is part of Bohrium and copyright (c) 2012 the Bohrium
team <http://www.bh107.org>.

Bohrium is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as 
published by the Free Software Foundation, either version 3 
of the License, or (at your option) any later version.

Bohrium is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the 
GNU Lesser General Public License along with Bohrium. 

If not, see <http://www.gnu.org/licenses/>.
*/
#endregion

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NumCIL.Float;
using NumCIL;
using T = System.Single;

namespace UnitTest
{
    public static class BasicTests
    {
        public static void RunTests()
        {
            var an = Generate.Empty(2000);
            var bn = Generate.Ones(2000);

            var cn = Generate.Empty(2000);
            var dn = Generate.Ones(2000);

            var an0 = an.Value[0];
            var bn0 = bn.Value[0];
            if (bn0 != 1)
                throw new Exception("Error in accessor");

            cn.Value[0] = 1;
            dn.Value[0] = 2;

            var cn0 = cn.Value[0];
            var dn0 = dn.Value[0];

            if (cn0 != 1 || dn0 != 2)
                throw new Exception("Error in accessor");

            var test = Generate.Range(3) * 4;
            test.Transpose();

            Shape s = new Shape(
                new long[] { 2, 1, 2, 3 },  //Dimension sizes
                6,                          //Offset
                new long[] { 18, 18, 6, 1 } //Strides
            );

            var a = Generate.Range(s.Length);
            var b = a.Reshape(s);
            var c = b[1][0][1];
            var d = c[2];
            var e = b.Flatten();
            if (e.AsArray().LongLength != 12 || e.AsArray()[3] != 12)
                throw new Exception("Failure in flatten");

            List<T> fln = new List<T>(b[1, 0, 1].Value);
            if (fln.Count != 3) throw new Exception("Failure in basic test 1");
            if (fln[0] != 30) throw new Exception("Failure in basic test 2");
            if (fln[1] != 31) throw new Exception("Failure in basic test 3");
            if (fln[2] != 32) throw new Exception("Failure in basic test 4");

            T n = b.Value[1, 0, 1, 2];
            if (n != 32) throw new Exception("Failure in basic test 5");
            if (c.Value[2] != 32) throw new Exception("Failure in basic test 6");
            if (c.Value[0] != 30) throw new Exception("Failure in basic test 7");
            if (c.Value[1] != 31) throw new Exception("Failure in basic test 8");
            if (b.Sum() != 228) throw new Exception("Failure in basic test 9");

            var r1 = Generate.Range(12).Reshape(new long[] { 2, 1, 2, 3 });
            if (!Equals(r1.AsArray(), new T[] { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 })) throw new Exception("Failure in basic test 10");
            var r2 = r1.Reduce<Add>(0);
            //if (!Equals(r2.AsArray(), new T[] { 6, 8, 10, 12, 14, 16 })) throw new Exception("Failure in basic test 11");
            r2 = r1.Reduce<Add>(1);
			var xxx = r2.AsArray();
			if (!Equals(xxx, new T[] { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 })) throw new Exception("Failure in basic test 12");

			r2 = r1.Reduce<Add>(2);
			var yyy = r2.AsArray();
			if (!Equals(yyy, new T[] { 3, 5, 7, 15, 17, 19 })) throw new Exception("Failure in basic test 13");

			//r2 = Add.Reduce(r1, 3);

			Console.WriteLine("Running problem test");
			r2 = r1.Reduce<Add>(3);
			var nnn = r2.AsArray();

			if (!Equals(nnn, new T[] { 3, 12, 21, 30 })) throw new Exception(string.Format("Failure in basic test 14: {0}", string.Join(", ",  nnn.Select(x => x.ToString()))));

            var r3 = b.Reduce<Add>();
            if (!Equals(r3.AsArray(), new T[] { 30, 32, 34, 42, 44, 46 })) throw new Exception("Failure in basic test 15");

            var x1 = Generate.Range(12).Reshape(new long[] { 4, 3 });
            var x2 = Generate.Range(3);

            var x3 = x1 + x2;

            var sqrd = x3.Sqrt();
            sqrd *= sqrd;
            sqrd = sqrd.Round();
            sqrd += 4;
            sqrd++;

            if (UFunc.Reduce<T, Add>(UFunc.Reduce<T, Add>(sqrd, 0)).Value[0] != 138) throw new Exception("Failure in basic arithmetics");
            if (UFunc.Reduce<T, Add>(UFunc.Reduce<T, Add>(sqrd, 1)).Value[0] != 138) throw new Exception("Failure in basic arithmetics");

            var x5 = sqrd.Apply((x) => x % 2 == 0 ? x : -x);
            if (!Equals(x5.AsArray(), new T[] { -5, -7, -9, 8, 10, 12, -11, -13, -15, 14, 16, 18 })) throw new Exception("Failure in basic test 16");

            NumCIL.UFunc.Apply<T, Add>(x1, x2, x3);
            NumCIL.Double.NdArray x4 = (NumCIL.Double.NdArray)x3;
            if (!Equals(x4.AsArray(), new double[] { 0, 2, 4, 3, 5, 7, 6, 8, 10, 9, 11, 13 })) throw new Exception("Failure in basic test 17");

            var x6 = Generate.Range(6).Reshape(new long[] { 2, 3 });

            var x7 = x6.Reduce<Add>();

            var rx7 = x7.AsArray();
            if (!Equals(rx7, new T[] { 3, 5, 7 })) throw new Exception(string.Format("Failure in basic test: [{0}]", string.Join(", ", rx7.Select(x => x.ToString()).ToArray())));

			var x8 = Generate.Range(10) * 0.5f;
            var rx8 = x8.Reduce<Add>().Value[0];
            if (rx8 != 22.5)
                throw new Exception(string.Format("Failure in broadcast multiply 1: {0}", rx8));

			var x9 = Mul.Apply(Generate.Range(10), 0.5f);
            var rx9 = x9.Reduce<Add>().Value[0];
            if (rx9 != 22.5)
                throw new Exception(string.Format("Failure in broadcast multiply 2: {0}", rx9));

            var x10 = 5 - Generate.Range(10);
            var x11 = Generate.Range(10) - 5;
            if (x10.Sum() != 5 || x11.Sum() != -5)
                throw new Exception("Failure in scalar rhs/lhs");

            var n0 = Generate.Range(4);
            var n1 = n0[new Range(1, 4)];
            var n2 = n0[new Range(0, 3)];
            var n3 = n1 - n2;
            if (n3.Reduce<Add>().Value[0] != 3)
                throw new Exception("Failure in basic slicing");

            var z0 = Generate.Range(new long[] {2, 2, 3});
            var z1 = z0[Range.All, Range.All, Range.El(1)];
            var z2 = z0[Range.All, Range.El(1), Range.El(1)];
            var z3 = z0[Range.El(1), Range.El(1), Range.All];
            var z4 = z0[Range.El(0), Range.El(1), Range.El(1)];
            var z5 = z0[Range.El(0)];

            if (z1.Shape.Elements != 4)
                throw new Exception("Reduced range failed");
            if (z2.Shape.Elements != 2 || z2.Shape.Dimensions.LongLength != 1)
                throw new Exception("Reduced range failed");
            if (z3.Shape.Elements != 3 || z3.Shape.Dimensions.LongLength != 1)
                throw new Exception("Reduced range failed");
            if (z4.Shape.Elements != 1 || z4.Shape.Dimensions.LongLength != 1)
                throw new Exception("Reduced range failed");
            if (z5.Shape.Elements != 6 || z5.Shape.Dimensions.LongLength != 2)
                throw new Exception("Reduced range failed");


            var y1 = Generate.Range(9).Reshape(new long[] {3, 3});
            var y2 = y1.Transposed;
            y1 = y1.Transposed;
            var y3 = y1 + y2;
            if (y3.Value[0, 2] != 12)
                throw new Exception("Failure with double transpose");

            var y4 = Generate.Range(2 * 2 * 2 * 2 * 2).Reshape(new long[] {2,2,2,2,2});
            var y5 = y4 * (Generate.Range(2) + 1);
            if (y5.Sum() != 752)
                throw new Exception("Failure with 5 dimensions");

            var y6 = Generate.Range(2 * 2 * 2 * 2 * 2 * 2).Reshape(new long[] { 2, 2, 2, 2, 2, 2 });
            var y7 = y6 * (Generate.Range(2) + 1);
            if (y7.Sum() != 3040)
                throw new Exception("Failure with 6 dimensions");

			var r = Generate.Random(2000);
			var rmin = Min.Aggregate(r);
			var rmax = Max.Aggregate(r);
			var ravg = Add.Aggregate(r) / r.DataAccessor.Length;

			if (rmin < 0 || rmax > 1.0)
				throw new Exception("Error in random");
			if (ravg < 0.4 || ravg > 0.6)
				throw new Exception("Deviation in random average");
        }

        private static bool Equals(double[] a, double[] b)
        {
            if (a.LongLength != b.LongLength)
                return false;

            for (long i = 0; i < a.LongLength; i++)
                if (a[i] != b[i])
                    return false;

            return true;
        }

        private static bool Equals(float[] a, float[] b)
        {
            if (a.LongLength != b.LongLength)
                return false;

            for (long i = 0; i < a.LongLength; i++)
                if (a[i] != b[i])
                    return false;

            return true;
        }
    }
}
