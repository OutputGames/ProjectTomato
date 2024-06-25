namespace TomatoScript.Tomato
{
    public class Actor : Object
    {
        public bool Enabled;

        public Transform Transform
        {
            get => _transform;
            internal set => _transform = value;
        }

        public string Name
        {
            get => _name;
            set
            {
                _name = value;
            }
        }

        public uint GetId() => _actorId;
        
        private uint _actorId;
        private string _name = "";
        private Transform _transform;
    }
}