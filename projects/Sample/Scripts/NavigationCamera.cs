using PhosEngine;

public class NavigationCamera : ScriptableEntity
{
    public float MovementSpeed = 1.0f;
    public float RotationSpeed = 2.0f;

    public override void OnCreate()
    {
    }

    public override void OnUpdate(float deltaTime)
    {
        var rotation = GetRotation();
        Transform.Rotate(rotation * RotationSpeed * deltaTime);

        var translation = GetTranslation();
        translation = Quaternion.FromEuler(Transform.Rotation) * translation;
        
        Transform.Translate(translation * MovementSpeed * deltaTime);
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

    private Vector3 GetRotation()
    {
        if (Input.IsMouseButtonDown(MouseButton.Right))
        {
            var horizontalChange = -Input.HorizontalAxisChange();
            var verticalChange = Input.VerticalAxisChange();

            return new Vector3(-verticalChange, horizontalChange, 0.0f);
        }

        return Vector3.Zero;
    }
}