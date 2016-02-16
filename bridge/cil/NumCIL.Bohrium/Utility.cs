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

namespace NumCIL.Bohrium
{
    /// <summary>
    /// Utility class for Bohrium
    /// </summary>
    public static class Utility
    {
        /// <summary>
        /// Helper class that provides a static destructor, which is not supported by CIL
        /// </summary>
        private class UnloaderHelper
        {
            /// <summary>
            /// Perform &quot;work&quot; so the compiler cannot optimize it away
            /// </summary>
            public void SetTime()
            {
            }

            /// <summary>
            /// Releases unmanaged resources and performs other cleanup operations before the
            /// <see cref="NumCIL.Bohrium.Utility+UnloaderHelper"/> is reclaimed by garbage collection.
            /// </summary>
            ~UnloaderHelper()
            {
                Utility.Flush();
            }
        }

        /// <summary>
        /// Static destructor helper
        /// </summary>
        private static UnloaderHelper _unloaderHelper = null;

        /// <summary>
        /// Attempts to set up Bohrium by looking for the Bohrium checkout folder.
        /// This simplifies using Bohrium directly from the build folder,
        /// without installing Bohrium first
        /// </summary>
        public static void SetupDebugEnvironmentVariables()
        {
            try
            {
                var allowednames = new string[] { "bohrium", "bohrium_priv", "bohrium-priv" };
                string basepath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
                Func<string, bool> eq = (x) =>
                {
                    foreach (var s in allowednames)
                        if (string.Equals(s, x, StringComparison.InvariantCultureIgnoreCase))
                            return true;
                    return false;
                };

                var root = System.IO.Path.GetPathRoot(basepath);
                while (basepath != root && !eq(System.IO.Path.GetFileName(basepath)))
                    basepath = System.IO.Path.GetDirectoryName(basepath);

                if (!eq(System.IO.Path.GetFileName(basepath)))
                {
                    basepath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
                    while (basepath != root && !System.IO.Directory.EnumerateFiles(basepath, "build.py").Any())
                        basepath = System.IO.Path.GetDirectoryName(basepath);

                    if (!System.IO.Directory.EnumerateFiles(basepath, "build.py").Any())
                        throw new Exception(string.Format("Unable to find a directory named {0}, in path {1}, searched until {2}", "'" + string.Join("', '", allowednames) + "'", System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location), basepath));
                }


                string binary_lookup_path = System.IO.Path.Combine(basepath, "core") + System.IO.Path.PathSeparator;

                //Bad OS detection :)
                if (System.IO.Path.PathSeparator == ':')
                {
                    bool isOsx = false;
                    try
                    {
                        isOsx = string.Equals("Darwin", System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo("uname") { UseShellExecute = false, RedirectStandardOutput = true }).StandardOutput.ReadToEnd().Trim());
                    }
                    catch { }

					string configpath = Environment.GetEnvironmentVariable("BOHRIUM_CONFIG") ?? "";
					if (string.IsNullOrEmpty(configpath))
					{
	                    if (isOsx)
						{
		                    string dyldpath = Environment.GetEnvironmentVariable("DYLD_LIBRARY_PATH") ?? "";
		                    Environment.SetEnvironmentVariable("DYLD_LIBRARY_PATH", binary_lookup_path + dyldpath);
                            Environment.SetEnvironmentVariable("BOHRIUM_CONFIG", System.IO.Path.Combine(basepath, "config.osx.ini"));
						}
	                    else
						{
		                    string ldpath = Environment.GetEnvironmentVariable("LD_LIBRARY_PATH") ?? "";
		                    Environment.SetEnvironmentVariable("LD_LIBRARY_PATH", binary_lookup_path + ldpath);
                            Environment.SetEnvironmentVariable("BOHRIUM_CONFIG", System.IO.Path.Combine(basepath, "config.ini"));
						}
					}
                }
                else
                {
                    binary_lookup_path += System.IO.Path.Combine(basepath, "pthread_win32");
                    string path = Environment.GetEnvironmentVariable("PATH");
                    Environment.SetEnvironmentVariable("PATH", path + System.IO.Path.PathSeparator + binary_lookup_path);
                    Environment.SetEnvironmentVariable("BOHRIUM_CONFIG", System.IO.Path.Combine(basepath, "config.win.ini"));
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to set up debug paths for Bohrium: " + ex.ToString());
            }
        }

        /// <summary>
        /// Activates Bohrium for all supported datatypes
        /// </summary>
        public static void Activate()
        {
            //Activate the instance so timings are more accurate when profiling
            //and also ensure that config problems are found during startup
            Flush();
            Activate<float>();
            Activate<double>();
            Activate<sbyte>();
            Activate<short>();
            Activate<int>();
            Activate<long>();
            Activate<byte>();
            Activate<ushort>();
            Activate<uint>();
            Activate<ulong>();
			Activate<bool>();
			Activate<NumCIL.Complex64.DataType>();
			Activate<System.Numerics.Complex>();
            
            NumCIL.UFunc.ApplyManager.RegisterHandler(new NumCIL.Bohrium.ApplyImplementor());
			_unloaderHelper = new UnloaderHelper();
            _unloaderHelper.SetTime();
        }

        /// <summary>
        /// Deactivates Bohrium for all supported datatypes
        /// </summary>
        public static void Deactivate()
        {
			_unloaderHelper = null;
            Flush();
            Deactivate<float>();
            Deactivate<double>();
            Deactivate<sbyte>();
            Deactivate<short>();
            Deactivate<int>();
            Deactivate<long>();
            Deactivate<byte>();
            Deactivate<ushort>();
            Deactivate<uint>();
            Deactivate<ulong>();
			Deactivate<bool>();
			Deactivate<NumCIL.Complex64.DataType>();
			Deactivate<System.Numerics.Complex>();

			GC.Collect();
			GC.WaitForPendingFinalizers();
        }
            
        /// <summary>
        /// Activates Bohrium for a specific datatype
        /// </summary>
        /// <typeparam name="T">The datatype to activate Bohrium for</typeparam>
		public static void Activate<T>()
        {
            NumCIL.Generic.NdArray<T>.AccessorFactory = new BohriumAccessorFactory<T>();
        }

        /// <summary>
        /// Deactivates Bohrium for a specific datatype
        /// </summary>
        /// <typeparam name="T">The datatype to deactivate Bohrium for</typeparam>
        public static void Deactivate<T>()
        {
            NumCIL.Generic.NdArray<T>.AccessorFactory = new NumCIL.Generic.DefaultAccessorFactory<T>();
        }

        /// <summary>
        /// Flushes pending operations in the VEM, note that this does not flush all pending instructions
        /// </summary>
        public static void Flush()
        {
            PinnedArrayTracker.ReleaseInternal();
        }

        /// <summary>
        /// Helper function that performs a memcpy from unmanaged data to managed data
        /// </summary>
        /// <param name="source">The data source</param>
        /// <param name="target">The data target</param>
        /// <typeparam name="T">The type of data to copy</typeparam>
        public static void WritePointerToArray<T>(IntPtr source, T[] target)
        {
            if (!UnsafeAPI.CopyFromIntPtr(source, target, target.Length))
            {
                if (typeof(T) == typeof(NumCIL.Complex64.DataType))
                {
                    var t = (NumCIL.Complex64.DataType[])(object)target;
                    var elsize = System.Runtime.InteropServices.Marshal.SizeOf(typeof(float));
                    var b = new byte[1000];
                    var rem = target.Length;
                    var offset = 0;
                    while (rem > 0)
                    {
                        var items = Math.Min(rem, 1000);
                        System.Runtime.InteropServices.Marshal.Copy(source, b, 0, items * elsize * 2);
                        var io = 0;
                        for (var i = 0; i < items; i++)
                        {
                            t[offset++] = new NumCIL.Complex64.DataType(
                                BitConverter.ToSingle(b, io),
                                BitConverter.ToSingle(b, io + elsize)
                            );
                            
                            io += (elsize * 2);
                        }
                        
                        source += items;
                        rem -= items;
                    }                    
                    return;
                }
                else if (typeof(T) == typeof(System.Numerics.Complex))
                {
                    var t = (System.Numerics.Complex[])(object)target;
                    var elsize = System.Runtime.InteropServices.Marshal.SizeOf(typeof(double));
                    var b = new byte[1000];
                    var rem = target.Length;
                    var offset = 0;
                    while (rem > 0)
                    {
                        var items = Math.Min(rem, 1000);
                        System.Runtime.InteropServices.Marshal.Copy(source, b, 0, items * elsize * 2);
                        var io = 0;
                        for (var i = 0; i < items; i++)
                        {
                            t[offset++] = new System.Numerics.Complex(
                                BitConverter.ToDouble(b, io),
                                BitConverter.ToDouble(b, io + elsize)
                            );
                            
                            io += (elsize * 2);
                        }
                        
                        source += items;
                        rem -= items;
                    }
                    
                    return;
                }
                else if (typeof(T) == typeof(byte))
                {
                    System.Runtime.InteropServices.Marshal.Copy(source, (byte[])(object)target, 0, target.Length);
                    return;
                }
                else if (typeof(T) == typeof(sbyte))
                {
                    var t = (sbyte[])(object)target;
                    var b = new byte[1000];
                    var rem = target.Length;
                    var offset = 0;
                    while (rem > 0)
                    {
                        var items = Math.Min(rem, 1000);
                        System.Runtime.InteropServices.Marshal.Copy(source, b, 0, items);
                        for(var i = 0; i < items; i++)
                            t[offset++] = (sbyte)b[i];
                        
                        source += items;
                        rem -= items;
                    }
                    return;
                }
                else if (typeof(T) == typeof(short))
                {
                    System.Runtime.InteropServices.Marshal.Copy(source, (short[])(object)target, 0, target.Length);
                    return;
                }
                else if (typeof(T) == typeof(ushort))
                {
                    var t = (ushort[])(object)target;
                    var elsize = System.Runtime.InteropServices.Marshal.SizeOf(typeof(ushort));
                    var b = new byte[1000 * elsize];
                    var rem = target.Length;
                    var offset = 0;
                    while (rem > 0)
                    {
                        var items = Math.Min(rem, 1000);
                        System.Runtime.InteropServices.Marshal.Copy(source, b, 0, items * elsize);
                        for(var i = 0; i < items; i++)
                            t[offset++] = BitConverter.ToUInt16(b, i * elsize);
                        
                        source += (items*elsize);
                        rem -= items;
                    }
                    
                    return;
                }
                else if (typeof(T) == typeof(int))
                {
                    System.Runtime.InteropServices.Marshal.Copy(source, (int[])(object)target, 0, target.Length);
                    return;
                }
                else if (typeof(T) == typeof(uint))
                {
                    var t = (uint[])(object)target;
                    var elsize = System.Runtime.InteropServices.Marshal.SizeOf(typeof(uint));
                    var b = new byte[1000 * elsize];
                    var rem = target.Length;
                    var offset = 0;
                    while (rem > 0)
                    {
                        var items = Math.Min(rem, 1000);
                        System.Runtime.InteropServices.Marshal.Copy(source, b, 0, items * elsize);
                        for(var i = 0; i < items; i++)
                            t[offset++] = BitConverter.ToUInt32(b, i * elsize);
                        
                        source += (items*elsize);
                        rem -= items;
                    }
                    
                    return;
                }
                else if (typeof(T) == typeof(long))
                {
                    System.Runtime.InteropServices.Marshal.Copy(source, (long[])(object)target, 0, target.Length);
                    return;
                }
                else if (typeof(T) == typeof(ulong))
                {
                    var t = (ulong[])(object)target;
                    var elsize = System.Runtime.InteropServices.Marshal.SizeOf(typeof(ulong));
                    var b = new byte[1000 * elsize];
                    var rem = target.Length;
                    var offset = 0;
                    while (rem > 0)
                    {
                        var items = Math.Min(rem, 1000);
                        System.Runtime.InteropServices.Marshal.Copy(source, b, 0, items * elsize);
                        for(var i = 0; i < items; i++)
                            t[offset++] = BitConverter.ToUInt64(b, i * elsize);
                        
                        source += (items*elsize);
                        rem -= items;
                    }
                    
                    return;
                }
                else if (typeof(T) == typeof(float))
                {
                    System.Runtime.InteropServices.Marshal.Copy(source, (float[])(object)target, 0, target.Length);
                    return;
                }
                else if (typeof(T) == typeof(double))
                {
                    System.Runtime.InteropServices.Marshal.Copy(source, (double[])(object)target, 0, target.Length);
                    return;
                }
                else if (typeof(T) == typeof(bool))
                {
                    var t = (bool[])(object)target;
                    var b = new byte[1000];
                    var rem = target.Length;
                    var offset = 0;
                    while (rem > 0)
                    {
                        var items = Math.Min(rem, 1000);
                        System.Runtime.InteropServices.Marshal.Copy(source, b, 0, items);
                        for(var i = 0; i < items; i++)
                            t[offset++] = b[i] != 0;
                        
                        source += items;
                        rem -= items;
                    }
                    
                    return;
                }
                
                throw new Exception("No copy performed");
            }
        }
        
        /// <summary>
        /// Helper function that performs a memcpy from unmanaged data to managed data
        /// </summary>
        /// <param name="source">The data source</param>
        /// <param name="target">The data target</param>
        /// <typeparam name="T">The type of data to copy</typeparam>
        public static void WriteArrayToPointer<T>(T[] source, IntPtr target)
        {
            if (!UnsafeAPI.CopyToIntPtr(source, target, source.Length))
            {
                if (typeof(T) == typeof(NumCIL.Complex64.DataType))
                {
                    var t = (NumCIL.Complex64.DataType[])(object)target;
                    var elsize = System.Runtime.InteropServices.Marshal.SizeOf(typeof(float));
                    for (var i = 0; i < source.Length; i++)
                    {
                        System.Runtime.InteropServices.Marshal.Copy(BitConverter.GetBytes(t[i].Real), 0, target, elsize); 
                        target += elsize;
                        System.Runtime.InteropServices.Marshal.Copy(BitConverter.GetBytes(t[i].Imaginary), 0, target, elsize); 
                        target += elsize;
                    }
   
                    return;
                }
                else if (typeof(T) == typeof(System.Numerics.Complex))
                {
                    var t = (System.Numerics.Complex[])(object)target;
                    var elsize = System.Runtime.InteropServices.Marshal.SizeOf(typeof(double));
                    for (var i = 0; i < source.Length; i++)
                    {
                        System.Runtime.InteropServices.Marshal.Copy(BitConverter.GetBytes(t[i].Real), 0, target, elsize); 
                        target += elsize;
                        System.Runtime.InteropServices.Marshal.Copy(BitConverter.GetBytes(t[i].Imaginary), 0, target, elsize); 
                        target += elsize;
                    }
                    
                    return;
                }
                else if (typeof(T) == typeof(byte))
                {
                    System.Runtime.InteropServices.Marshal.Copy((byte[])(object)source, 0, target, source.Length);
                    return;
                }
                else if (typeof(T) == typeof(sbyte))
                {
                    var t = (sbyte[])(object)target;
                    for (var i = 0; i < source.Length; i++)
                    {
                        System.Runtime.InteropServices.Marshal.Copy(BitConverter.GetBytes(t[i]), 0, target, 1); 
                        target += 1;
                    }

                }
                else if (typeof(T) == typeof(short))
                {
                    System.Runtime.InteropServices.Marshal.Copy((short[])(object)source, 0, target, source.Length);
                    return;
                }
                else if (typeof(T) == typeof(ushort))
                {
                    var t = (ushort[])(object)target;
                    var elsize = System.Runtime.InteropServices.Marshal.SizeOf(typeof(ushort));
                    for (var i = 0; i < source.Length; i++)
                    {
                        System.Runtime.InteropServices.Marshal.Copy(BitConverter.GetBytes(t[i]), 0, target, elsize); 
                        target += elsize;
                    }
                    
                    return;
                }
                else if (typeof(T) == typeof(int))
                {
                    System.Runtime.InteropServices.Marshal.Copy((int[])(object)source, 0, target, source.Length);
                    return;
                }
                else if (typeof(T) == typeof(uint))
                {
                    var t = (uint[])(object)target;
                    var elsize = System.Runtime.InteropServices.Marshal.SizeOf(typeof(uint));
                    for (var i = 0; i < source.Length; i++)
                    {
                        System.Runtime.InteropServices.Marshal.Copy(BitConverter.GetBytes(t[i]), 0, target, elsize); 
                        target += elsize;
                    }

                    
                    return;
                }
                else if (typeof(T) == typeof(long))
                {
                    System.Runtime.InteropServices.Marshal.Copy((long[])(object)source, 0, target, source.Length);
                    return;
                }
                else if (typeof(T) == typeof(ulong))
                {
                    var t = (ulong[])(object)target;
                    var elsize = System.Runtime.InteropServices.Marshal.SizeOf(typeof(ulong));
                    for (var i = 0; i < source.Length; i++)
                    {
                        System.Runtime.InteropServices.Marshal.Copy(BitConverter.GetBytes(t[i]), 0, target, elsize); 
                        target += elsize;
                    }
                    
                    return;
                }
                else if (typeof(T) == typeof(float))
                {
                    System.Runtime.InteropServices.Marshal.Copy((float[])(object)source, 0, target, source.Length);
                    return;
                }
                else if (typeof(T) == typeof(double))
                {
                    System.Runtime.InteropServices.Marshal.Copy((double[])(object)source, 0, target, source.Length);
                    return;
                }
                else if (typeof(T) == typeof(bool))
                {
                    var t = (bool[])(object)target;
                    for (var i = 0; i < source.Length; i++)
                    {
                        System.Runtime.InteropServices.Marshal.WriteByte(target, t[i] ? (byte)1 : (byte)0); 
                        target += 1;
                    }

                    
                    return;
                }
                
                throw new Exception("No copy performed");
            }
        }
        
    }
}
