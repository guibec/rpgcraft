using UnityEngine;

public class Slime : Enemy
{
    public float m_speed = 3f;
    public float m_idleTime = 5.0f;
    public float m_seekTime = 3.0f;

    private float m_time;

    public float m_doubleHeartSpawnChance = 0.05f;
    public float m_heartSpawnChance = 0.10f;

    public Slime()
    {

    }

    private EntityRender m_er;

    private readonly string[] frameGroups = 
    {
        "Left",
        "Down",
        "Right",
        "Up",
    };

    public enum State
    {
        E_Idle,
        E_Seeking,
    };

    private const int Left_Cycle1 = 0;
    private const int Left_Cycle2 = 1;
    private const int Left_Cycle3 = 2;
    private const int Down_Cycle1 = 0;
    private const int Down_Cycle2 = 1;
    private const int Down_Cycle3 = 2;
    private const int Right_Cycle1 = 0;
    private const int Right_Cycle2 = 1;
    private const int Right_Cycle3 = 2;
    private const int Up_Cycle1 = 0;
    private const int Up_Cycle2 = 1;
    private const int Up_Cycle3 = 2;

    public State CurrentState { get; private set; }

    protected override void OnAwake()
    {
        base.OnAwake();

        m_er = GetComponent<EntityRender>();
        CurrentState = State.E_Idle;
        m_time = 0;
    }

    protected override void OnStart()
    {
        base.OnStart();

        if (m_er)
        {
            for (int j = 0; j < 4; ++j)
            {
                for (int i = 0; i < 3; ++i)
                {
                    m_er.SetFrameInfo(frameGroups[j], i * 16, j * 16, 16, 16);        
                }
            }

            // now link frames together
            m_er.LinkNextFrame(frameGroups[0], Up_Cycle1, Up_Cycle2);
            m_er.LinkNextFrame(frameGroups[0], Up_Cycle2, Up_Cycle3);
            m_er.LinkNextFrame(frameGroups[0], Up_Cycle3, Up_Cycle1);

            m_er.LinkNextFrame(frameGroups[1], Right_Cycle1, Right_Cycle2);
            m_er.LinkNextFrame(frameGroups[1], Right_Cycle2, Right_Cycle3);
            m_er.LinkNextFrame(frameGroups[1], Right_Cycle3, Right_Cycle1);

            m_er.LinkNextFrame(frameGroups[2], Down_Cycle1, Down_Cycle2);
            m_er.LinkNextFrame(frameGroups[2], Down_Cycle2, Down_Cycle3);
            m_er.LinkNextFrame(frameGroups[2], Down_Cycle3, Down_Cycle1);

            m_er.LinkNextFrame(frameGroups[3], Left_Cycle1, Left_Cycle2);
            m_er.LinkNextFrame(frameGroups[3], Left_Cycle2, Left_Cycle3);
            m_er.LinkNextFrame(frameGroups[3], Left_Cycle3, Left_Cycle1);

            m_er.SetGlobalFrameDelay(0.2f);

            m_er.SetCurrentGroup(frameGroups[RandomManager.Next(0, frameGroups.Length)]);

        }
    }

    // TODO: We will want a some point to distinguish when an Entity is killed a "gameplay" event
    // and when an Entity is destroyed. When it is killed, we want to give rewards.
    // when it is destroyed, we may not want to, since there could be other reasons (level ended, some cutscene removed all enemies, etc.)
    protected override void OnEntityDestroy()
    {
        base.OnEntityDestroy();
    }

    protected override void OnUpdate()
    {
        base.OnUpdate();

        m_time += TimeManager.Dt;
        switch (CurrentState)
        {
            case State.E_Idle:
                if (m_time >= m_idleTime)
                {
                    CurrentState = State.E_Seeking;
                    m_time = 0;
                }
                break;
            case State.E_Seeking:
                if (m_time >= m_seekTime)
                {
                    CurrentState = State.E_Idle;
                    m_time = 0;
                }
                break;
        }

        if (CurrentState == State.E_Idle)
        {
            return;
        }

        // just go toward the main player
        Vector2 playerPos = GameManager.Instance.MainPlayer.transform.position;

        string currentGroup = m_er ? m_er.GetCurrentGroup() : "";

        string newFrameGroup = "Up";
        bool setFrame = false;
        Vector2 wantedNewPos = transform.position;

        // we prefer to continue in direction we are going if possible
        // there are 4 possible directions, but basically horizontal vs vertical
        bool preferVertical;
        if (currentGroup == "Up" || currentGroup == "Down")
            preferVertical = true;
        else if (currentGroup == "Left" || currentGroup == "Right")
            preferVertical = false;
        else
        {
            preferVertical = RandomManager.Boolean();
            Debug.Log("Slime perfer vertical is " + preferVertical);
        }

        // TODO: Extract into functions to avoid this copy pasta between preferVertical and non perferVertical
        if (preferVertical)
        {
            if (playerPos.y > transform.position.y)
            {
                wantedNewPos.y = transform.position.y + (TimeManager.Dt * m_speed);
                if (wantedNewPos.y > playerPos.y)
                    wantedNewPos.y = playerPos.y;

                newFrameGroup = "Up";
                setFrame = true;
            }
            else if (playerPos.y < transform.position.y)
            {
                wantedNewPos.y = transform.position.y - (TimeManager.Dt * m_speed);
                if (wantedNewPos.y < playerPos.y)
                    wantedNewPos.y = playerPos.y;

                newFrameGroup = "Down";
                setFrame = true;
            }
            else if (playerPos.x < transform.position.x)
            {
                wantedNewPos.x = transform.position.x - (TimeManager.Dt * m_speed);
                if (wantedNewPos.x < playerPos.x)
                    wantedNewPos.x = playerPos.x;

                newFrameGroup = "Left";
                setFrame = true;
            }
            else if (playerPos.x > transform.position.x)
            {
                wantedNewPos.x = transform.position.x + (TimeManager.Dt * m_speed);
                if (wantedNewPos.x > playerPos.x)
                    wantedNewPos.x = playerPos.x;

                newFrameGroup = "Right";
                setFrame = true;
            }
        }
        else
        {
            if (playerPos.x < transform.position.x)
            {
                wantedNewPos.x = transform.position.x - (TimeManager.Dt * m_speed);
                if (wantedNewPos.x < playerPos.x)
                    wantedNewPos.x = playerPos.x;

                newFrameGroup = "Left";
                setFrame = true;
            }
            else if (playerPos.x > transform.position.x)
            {
                wantedNewPos.x = transform.position.x + (TimeManager.Dt * m_speed);
                if (wantedNewPos.x > playerPos.x)
                    wantedNewPos.x = playerPos.x;

                newFrameGroup = "Right";
                setFrame = true;
            }
            else if (playerPos.y > transform.position.y)
            {
                wantedNewPos.y = transform.position.y + (TimeManager.Dt * m_speed);
                if (wantedNewPos.y > playerPos.y)
                    wantedNewPos.y = playerPos.y;

                newFrameGroup = "Up";
                setFrame = true;
            }
            else if (playerPos.y < transform.position.y)
            {
                wantedNewPos.y = transform.position.y - (TimeManager.Dt * m_speed);
                if (wantedNewPos.y < playerPos.y)
                    wantedNewPos.y = playerPos.y;

                newFrameGroup = "Down";
                setFrame = true;
            }

        }

        transform.position = wantedNewPos;

        if (setFrame && m_er)
        {
            m_er.SetCurrentGroup(newFrameGroup);
        }
    }

    protected override void OnLateUpdate()
    {
        base.OnLateUpdate();
    }
}
