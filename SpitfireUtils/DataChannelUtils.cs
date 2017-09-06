using System;
using System.Collections.Generic;
using System.Text;

namespace SpitfireUtils
{
    public static class DataChannelUtils
    {

        /// <summary>
        /// A memcpy based variant of the Resize function, it is used to copy a segment 
        /// into a new buffer, usually smaller.
        /// </summary>
        /// <param name="source"></param>
        /// <param name="target"></param>
        /// <param name="count"></param>
        public static unsafe void MemoryResize(ref byte[] source, ref byte[] target, uint count)
        {
            fixed (byte* pSource = source, pTarget = target)
            {
                var ps = pSource;
                var pt = pTarget;
                Native.Copy(pt, ps, count);
            }
        }
        /// <summary>
        /// Uses memcpy to slice an array into smaller chunks very quickly. 
        /// </summary>
        /// <param name="source"></param>
        /// <param name="sourceOffset"></param>
        /// <param name="target"></param>
        /// <param name="targetOffset"></param>
        /// <param name="count"></param>
        public static unsafe void MemorySlice(ref byte[] source, int sourceOffset, ref byte[] target, int targetOffset, uint count)
        {
            fixed (byte* pSource = source, pTarget = target)
            {
                var ps = pSource + sourceOffset;
                var pt = pTarget + targetOffset;
                Native.Copy(pt, ps, count);
            }
        }

        /// <summary>
        /// Copy a segment of data into a new array using pointers.
        /// </summary>
        /// <param name="source"></param>
        /// <param name="target"></param>
        /// <param name="count"></param>
        public static unsafe void Resize(ref byte[] source, ref byte[] target, int count)
        {
            fixed (byte* pSource = source, pTarget = target)
            {
                var ps = pSource;
                var pt = pTarget;
                for (var i = 0; i < count; i++)
                {
                    *pt = *ps;
                    pt++;
                    ps++;
                }
            }
        }
        /// <summary>
        /// Slice an array into smaller chunks using pointers.
        /// </summary>
        /// <param name="source"></param>
        /// <param name="sourceOffset"></param>
        /// <param name="target"></param>
        /// <param name="targetOffset"></param>
        /// <param name="count"></param>
        public static unsafe void Slice(ref byte[] source, int sourceOffset, ref byte[] target, int targetOffset, uint count)
        {
            fixed (byte* pSource = source, pTarget = target)
            {
                var ps = pSource + sourceOffset;
                var pt = pTarget + targetOffset;
                for (var i = 0; i < count; i++)
                {
                    *pt = *ps;
                    pt++;
                    ps++;
                }
            }
        }
    }
}
