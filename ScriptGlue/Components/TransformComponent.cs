namespace PhosEngine
{
    public class TransformComponent : Component
    {
        public Vector3 Position
        {
            get
            {
                InternalCalls.TransformComponent_GetPosition(Entity.Id, out var val);
                return val;
            }
            set => InternalCalls.TransformComponent_SetPosition(Entity.Id, ref value);
        }

        public Vector3 Scale
        {
            get
            {
                InternalCalls.TransformComponent_GetScale(Entity.Id, out var val);
                return val;
            }
            set => InternalCalls.TransformComponent_SetScale(Entity.Id, ref value);
        }

        public Vector3 Rotation
        {
            get
            {
                InternalCalls.TransformComponent_GetRotation(Entity.Id, out var val);
                return val;
            }
            set => InternalCalls.TransformComponent_SetRotation(Entity.Id, ref value);
        }

        public void Translate(float x, float y, float z)
        {
            Position = Position + new Vector3(x, y, z);
        }
    }
}