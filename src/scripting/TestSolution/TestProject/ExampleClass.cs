using PhosEngine;

public class ExampleClass : Entity
{
    public float MyPublicFloatVar = 0.0f;

    public override void OnCreate()
    {
        MyPublicFloatVar = 10.0f;
    }

    public override void OnUpdate()
    {
        InternalCalls.Get_Position(Id, out Vector3 pos);
        MyPublicFloatVar = pos.Z;
    }
}