namespace TomatoScript.Tomato {
    public abstract class Component : Object
    {
        private Actor _actor;

        public Actor Actor
        {
            get => _actor;
            internal set => _actor = value;
        }


        protected virtual void Start()
        {
            
        }

        protected virtual void Update()
        {
            
        }
    }
}