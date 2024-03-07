using PhosEngine;

public class NavigationCamera : ScriptableEntity
{
    public float Speed = 1.0f;

    public override void OnCreate()
    {
    }

    public override void OnUpdate(float deltaTime)
    {
        var translation = GetTranslation();
        Transform.Translate(translation * Speed * deltaTime);

        if (Input.IsMouseButtonDown(MouseButton.Right))
        {
            var horizontalChange = -Input.HorizontalAxisChange();
            var verticalChange = Input.VerticalAxisChange();

            Transform.Rotate(horizontalChange * deltaTime, verticalChange * deltaTime, 0.0f);
        }
    }

    private Vector3 GetTranslation()
    {
        var translation = Vector3.Zero;
        if (Input.IsKeyDown(Key.W))
        {
            translation += new Vector3(0.0f, 0.0f, -1.0f);
        }

        if (Input.IsKeyDown(Key.S))
        {
            translation += new Vector3(0.0f, 0.0f, 1.0f);
        }

        if (Input.IsKeyDown(Key.A))
        {
            translation += new Vector3(-1.0f, 0.0f, 0.0f);
        }

        if (Input.IsKeyDown(Key.D))
        {
            translation += new Vector3(1.0f, 0.0f, 0.0f);
        }

        return translation;
    }
}