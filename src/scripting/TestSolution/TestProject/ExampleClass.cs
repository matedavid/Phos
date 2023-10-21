public class ExampleClass
{
    public float MyPublicFloatVar = 0.0f;

    public void OnCreate()
    {
        MyPublicFloatVar = 10.0f;
    }

    private void OnUpdate()
    {
        MyPublicFloatVar += 1;
    }
}