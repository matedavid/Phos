namespace PhosEngine
{
    public abstract class Component
    {
        internal ScriptableEntity Entity;

        public Component() => Entity = null;
        public Component(ScriptableEntity entity) => Entity = entity;
    }
}