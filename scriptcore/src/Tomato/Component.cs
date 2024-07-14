namespace TomatoScript.Tomato {
    public abstract class Component : Object
    {
        private Actor _actor;
        private Transform _transform;

        public Actor Actor
        {
            get => _actor;
            internal set => _actor = value;
        }

        public Transform Transform
        {
            get => _transform;
            internal set => _transform = value;
        }


        protected virtual void Start()
        {
            
        }

        protected virtual void Update()
        {
            
        }
    }
}