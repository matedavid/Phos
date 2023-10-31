namespace PhosEngine
{
    public static class Input
    {
        public static bool IsKeyDown(Key key)
        {
            InternalCalls.Input_IsKeyDown((uint)key, out var isDown);
            return isDown;
        }

        public static bool IsMouseButtonDown(MouseButton mouseButton)
        {
            InternalCalls.Input_IsMouseButtonDown((uint)mouseButton, out var isDown);
            return isDown;
        }
    }
}