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
        // MyPublicFloatVar += 1;
        MyPublicFloatVar = InternalCalls.Sample();
    }
}