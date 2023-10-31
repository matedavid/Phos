namespace PhosEngine
{
    public abstract class Component
    {
        internal Entity Entity;

        public Component() => Entity = null;
        public Component(Entity entity) => Entity = entity;
    }
}