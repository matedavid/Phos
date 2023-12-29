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
            Translate(new Vector3(x, y, z));
        }

        public void Translate(Vector3 vec)
        {
            Position += vec;
        }

        public void Rotate(float x, float y, float z)
        {
            Rotate(new Vector3(x, y, z));
        }

        public void Rotate(Vector3 vec)
        {
            Rotation += vec;
        }
    }
}