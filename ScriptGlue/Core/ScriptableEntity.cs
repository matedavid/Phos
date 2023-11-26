using System.Runtime.InteropServices;

namespace PhosEngine
{
    [StructLayout(LayoutKind.Sequential)]
    public class ScriptableEntity
    {
        public readonly ulong Id;

        public ScriptableEntity() => Id = 0;

        public ScriptableEntity(ulong id) => Id = id;

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
            component.Entity = this;

            return component;
        }

        protected Entity Instantiate(Prefab prefab)
        {
            InternalCalls.Entity_Instantiate(prefab.Id, out var id);
            return new Entity() { Id = id };
        }

        protected void Destroy(Entity entity)
        {
            InternalCalls.Entity_Destroy(entity.Id);
        }
    }
}