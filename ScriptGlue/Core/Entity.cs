using System;

namespace PhosEngine
{
    public class Entity
    {
        internal readonly ulong Id;

        public Entity() => Id = 0;
        public Entity(ulong id) => Id = id;

        public virtual void OnCreate()
        {
        }

        public virtual void OnUpdate()
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

        protected Entity Instantiate(ulong prefabId)
        {
            InternalCalls.Entity_Instantiate(prefabId, out var id);
            return new Entity(id);
        }

        protected void Destroy(Entity entity)
        {
            InternalCalls.Entity_Destroy(entity.Id);
        }
    }
}