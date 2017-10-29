class Mouse
{
    public:
        
        void update();
        
		bool hasFocus() const { return m_scene_has_focus; }
        bool isInScene() const { return m_mouse_in_scene; }
        float2 getRelativeToCenter() const { return m_mouse_pos_relative_to_center; }
        
    protected:
    private:
    
    bool	m_scene_has_focus = false;
    bool	m_mouse_in_scene = false;
    float2	m_mouse_pos_relative_to_center = {};
};