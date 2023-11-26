using System.Runtime.InteropServices;

namespace PhosEngine
{
    [StructLayout(LayoutKind.Sequential)]
    public class ScriptableEntity
    {
        public readonly ulong Id;

        public ScriptableEntity() => Id = 0;

        public ScriptableEntity(ulong id)
        {
            Id = id;
            Logging.Info($"Entity id: {Id}");
        }

        public virtual void OnCreate()
        {
        }

        public virtual void OnUpdate(float deltaTime)
        {
        }

        public TransformComponent Transform => GetComponent<TransformComponent>();

        private T GetComponent<T>()
            where T : Component, new()
        {
            var component = new T();
            component.ScriptableEntity = this;

            return component;
        }

        protected ScriptableEntity Instantiate(Prefab prefab)
        {
            InternalCalls.Entity_Instantiate(prefab.Id, out var id);
            return new ScriptableEntity(id);
        }

        protected void Destroy(ScriptableEntity entity)
        {
            InternalCalls.Entity_Destroy(entity.Id);
        }
    }
}