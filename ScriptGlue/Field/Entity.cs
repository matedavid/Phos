using System.Runtime.InteropServices;

namespace PhosEngine
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Entity
    {
        public ulong Id;

        private ScriptableEntity _entity => new ScriptableEntity(Id);

        public TransformComponent Transform => _entity.Transform;
    }
}