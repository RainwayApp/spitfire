using System.Reflection.Emit;

namespace SpitfireUtils
{

    internal static class Native
    {
        public unsafe delegate void MemCpyFunction(void* des, void* src, uint bytes);

        public static MemCpyFunction Copy;

        static Native()
        {
            var dynamicMethod = new DynamicMethod
            (
                "MemCpy",
                typeof(void),
                new[] { typeof(void*), typeof(void*), typeof(uint) },
                typeof(Native)
            );

            var ilGenerator = dynamicMethod.GetILGenerator();

            ilGenerator.Emit(OpCodes.Ldarg_0);
            ilGenerator.Emit(OpCodes.Ldarg_1);
            ilGenerator.Emit(OpCodes.Ldarg_2);

            ilGenerator.Emit(OpCodes.Cpblk);
            ilGenerator.Emit(OpCodes.Ret);

            Copy = (MemCpyFunction)dynamicMethod.CreateDelegate(typeof(MemCpyFunction));
        }
    }
}
