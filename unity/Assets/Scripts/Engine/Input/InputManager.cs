using UnityEngine;
using System.Collections;

public class InputManager : MonoSingleton<InputManager>
{
    public Camera m_camera;

    public float m_speed = 1f; // in tile per second

    protected override void OnUpdate()
    {
        base.OnUpdate();

        Vector3 direction = Vector3.zero;

        if (Input.GetKey(KeyCode.UpArrow))
            direction.y = m_speed;
        else if (Input.GetKey(KeyCode.DownArrow))
            direction.y = -m_speed;

        if (Input.GetKey(KeyCode.LeftArrow))
            direction.x = -m_speed;
        else if (Input.GetKey(KeyCode.RightArrow))
            direction.x = m_speed;

        direction *= TimeManager.Dt;
        //m_camera.transform.position = m_camera.transform.position + direction;

        OnMouseUpdate();
    }

    private void OnMouseUpdate()
    {
        Vector2 screenPos = Input.mousePosition;
        Vector3 worldPos = Camera.main.ScreenToWorldPoint(screenPos);

        ItemInstance itemInstance = GameManager.Instance.GetItemInstanceFromWorldPos(worldPos);

        if (itemInstance)
        {
            UIManager.Instance.UpdateMouseToolTip(screenPos, itemInstance.Item.ToString());
        }
        else
        {
            Enemy enemy = GameManager.Instance.GetEnemyFromWorldPos(worldPos);
            UIManager.Instance.UpdateMouseToolTip(screenPos, enemy ? enemy.GetType().Name : "");
        }
    }
}
