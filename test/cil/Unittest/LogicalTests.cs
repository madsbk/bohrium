#region Copyright
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

namespace UnitTest
{
	public static class LogicalTests
	{
		public static void RunTests()
		{
			var a = NumCIL.Float.Generate.Range(10);
			var b = NumCIL.Float.Generate.Ones(10);
			var c = a > 5;
			var d = 5 < a;
			var e = a == b;
			var f = (NumCIL.UInt8.NdArray)c;
			var g = (NumCIL.UInt8.NdArray)d;
			var h = (NumCIL.UInt8.NdArray)e;

			var fsum = f.Sum();
			if (f.Sum() != 4)
				throw new Exception(string.Format("Error in compare 1: {0}", fsum));
			if (g.Sum() != 4)
				throw new Exception("Error in compare 2");
			if (h.Sum() != 1)
				throw new Exception("Error in compare 3");

			var i = (NumCIL.UInt8.NdArray)(c ^ !e);
			var j = (NumCIL.UInt8.NdArray)(c & !e);
			var k = (NumCIL.UInt8.NdArray)(c | e);
			if (i.Sum() != 5 || j.Sum() != 4 || k.Sum() != 5)
				throw new Exception("Error in logical operation");

			var l = (NumCIL.UInt8.NdArray)((a >= 4) | (a <= 2));
			if (l.Sum() != 9)
				throw new Exception("Error in compare 4");

		}
	}
}

