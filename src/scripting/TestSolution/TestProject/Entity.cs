namespace PhosEngine
{
    public abstract class Entity
    {
        public readonly ulong Id;

        public Entity()
        {
            Id = 0;
        }

        public Entity(ulong id)
        {
            Id = id;
        }

        public abstract void OnCreate();
        public abstract void OnUpdate();
    }
}