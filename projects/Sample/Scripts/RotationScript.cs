using PhosEngine;

public class RotationScript : ScriptableEntity
{
    public float Speed;

    public override void OnCreate()
    {
    }

    public override void OnUpdate(float deltaTime)
    {
        var rotation = new Vector3(0.0f, Speed * deltaTime, 0.0f);
        Transform.Rotate(rotation);
    }
}
