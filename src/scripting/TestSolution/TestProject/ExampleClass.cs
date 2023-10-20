public class ExampleClass
{
    public float MyPublicFloatVar = 5.0f;

    public void SetFloatValue(float value)
    {
        MyPublicFloatVar = value;
    }

    private void IncrementFloatVar()
    {
        MyPublicFloatVar += 1;
    }
}