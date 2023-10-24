using System.Runtime.Remoting.Metadata.W3cXsd2001;
using PhosEngine;

namespace PhosEngine
{
    public abstract class Entity
    {
        internal readonly ulong Id;

        public Entity() => Id = 0;
        public Entity(ulong id) => Id = id;

        public abstract void OnCreate();
        public abstract void OnUpdate();

        public TransformComponent Transform => GetComponent<TransformComponent>();

        private T GetComponent<T>()
            where T : Component, new()
        {
            var component = new T();
            component.Entity = this;

            return component;
        }
    }
}