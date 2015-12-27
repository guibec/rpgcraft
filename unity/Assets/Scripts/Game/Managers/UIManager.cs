using System;
using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using UnityEngine.UI;
using IronExtension;

public class UIManager : MonoSingleton<UIManager>
{
    // Prefabs & settings
    public GameObject m_txtDynamicPrefab;

    private struct InventorySlotUI
    {
        public int m_index;
        public GameObject m_parent;
        public Button m_button;
        public Text m_title;
        public Text m_count;
        public Image m_icon;

        public static InventorySlotUI CreateFromGameObject(GameObject parent_, int index)
        {
            InventorySlotUI isu;
            isu.m_index = index;
            isu.m_parent = parent_;
            isu.m_button = parent_.GetComponent<Button>();
            isu.m_title = Helpers.FindChildWithName(parent_, "txtSlot").GetComponent<Text>();
            isu.m_count = Helpers.FindChildWithName(parent_, "txtCount").GetComponent<Text>();
            isu.m_icon = Helpers.FindChildWithName(parent_, "imgIcon").GetComponent<Image>();

            return isu;
        }
    }

    public class DynamicText
    {
        public GameObject Object { get; set; }
        public ScreenFollowWorld ScreenFollow;
        public float DisplayTime;

        public void ParentToWorld(GameObject worldObject)
        {
            if (ScreenFollow == null)
            {
                ScreenFollow = Object.GetComponent<ScreenFollowWorld>();
            }

            if (ScreenFollow == null)
                return;

            ScreenFollow.Target = worldObject;
        }

        // simple mover functionality here
        // if it gets too big, move out of this class and make a new class instead
        private Vector2 Displacement;
        private float DisplacementTimeLeft;

        public void SimpleMovement(Vector2 displacement, float time)
        {
            Displacement = displacement;
            DisplacementTimeLeft = time;
        }

        public void Update()
        {
            if (DisplacementTimeLeft >= 0)
            {
                float previousTime = DisplacementTimeLeft;
                DisplacementTimeLeft -= TimeManager.Dt;
                DisplacementTimeLeft = Mathf.Max(DisplacementTimeLeft, 0);

                float currentDt = previousTime - DisplacementTimeLeft;

                if (ScreenFollow != null)
                {
                    ScreenFollow.Offset += Displacement * currentDt;
                }
                else
                {
                    Object.transform.Translate(Displacement * currentDt);
                }
            }
        }
    }

    private const int NumInventorySlots = 10;

    private readonly List<InventorySlotUI> m_inventorySlot = new List<InventorySlotUI>(NumInventorySlots);
    private Text m_healthText;
    private Slider m_xpSlider;
    private Text m_txtLevel;

    public Text m_mouseTileInfo;

    private GameObject m_canvas;

    private List<DynamicText> m_dynamicTexts = new List<DynamicText>(10);

    private bool m_initialized = false;

    protected override void Awake()
    {
        base.Awake();

        GameObject masterSlot = GameObject.Find("Canvas/btn_inv");
        m_canvas = GameObject.Find("Canvas");

        if (m_canvas == null)
            UnityEngine.Debug.LogWarning("Could not find canvas");

        if (masterSlot == null)
            UnityEngine.Debug.LogWarning("Could not find inventory slot");
        else
        {
            m_inventorySlot.Add(InventorySlotUI.CreateFromGameObject(masterSlot, 0));
            SetupEvents(m_inventorySlot[0]);
        }

        m_healthText = GameObject.Find("Canvas/txt_Health").GetComponent<Text>();
        m_mouseTileInfo = GameObject.Find("Canvas/txt_MouseCursorHover").GetComponent<Text>();
        m_xpSlider = GameObject.Find("Canvas/sld_XP").GetComponent<Slider>();
        m_txtLevel = GameObject.Find("Canvas/txt_Level").GetComponent<Text>();
    }

    private int m_selectSlotIndex = 0;
    public int SelectedInventorySlot
    {
        get
        {
            return m_selectSlotIndex;
        }
        set
        {
            m_selectSlotIndex = value;
            UpdateText();
        }
    }

    public void Start()
    {
        // listen to invents from inventory
        if (GameManager.Instance != null)
        {
            GameManager.Instance.MainPlayer.Inventory.Changed +=
                new Inventory.InventoryChangedEventHandler(OnInventoryChanged);

            GameManager.Instance.MainPlayer.HealthComponent.HealthChanged +=
                new HealthComponent.HealthChangedEventHandler(OnHealthChanged);

            GameManager.Instance.MainPlayer.Experience.Changed +=
                new Experience.XPChangedEventHandler(OnXPChanged);


            UpdateHealth();
        }
    }

    protected override void OnUpdate()
    {
        base.OnUpdate();

        if (!m_initialized)
            Duplicate();

        HandleDynamicText();
    }

    private void HandleDynamicText()
    {
        for (int i = m_dynamicTexts.Count - 1; i >= 0; i--)
        {
            DynamicText textObject = m_dynamicTexts[i];
            textObject.Update();

            textObject.DisplayTime -= TimeManager.Dt;
            if (textObject.DisplayTime <= 0)
            {
                DestroyObject(textObject.Object);
                m_dynamicTexts.RemoveAt(i);
            }
        }
    }

    protected override void OnInit()
    {
        base.OnInit();

        UnityEngine.Debug.Log("UIManager::Init");
    }

    public void OnDestroy()
    {
        if (GameManager.Instance != null)
        {
            GameManager.Instance.MainPlayer.Inventory.Changed -= new Inventory.InventoryChangedEventHandler(OnInventoryChanged);
            GameManager.Instance.MainPlayer.HealthComponent.HealthChanged -= new HealthComponent.HealthChangedEventHandler(OnHealthChanged);
            GameManager.Instance.MainPlayer.Experience.Changed -=
                new Experience.XPChangedEventHandler(OnXPChanged);
        }
    }

    public void OnInventoryChanged(object sender, EventArgs e)
    {
        UpdateText();
    }

    public void OnHealthChanged(object sender, EventArgs e)
    {
        UpdateHealth();
    }

    public void OnXPChanged(object sender, EventArgs e)
    {
        UpdateXP();
    }

    public void UpdateMouseToolTip(Vector2 screenPos, string text)
    {
        m_mouseTileInfo.transform.position = screenPos;
        m_mouseTileInfo.text = text;
    }

    private void UpdateHealth()
    {
        if (!GameManager.Instance)
            return;

        m_healthText.text = GameManager.Instance.MainPlayer.HealthComponent.Health.ToString();
    }

    private void UpdateXP()
    {
        if (!GameManager.Instance)
            return;

        int XPRequired = GameManager.Instance.MainPlayer.Experience.GetXPRequiredForNextLevel();
        if (XPRequired == int.MaxValue)
        {
            m_xpSlider.value = 0f;
        }
        else
        {
            m_xpSlider.value = (float) GameManager.Instance.MainPlayer.Experience.XP/XPRequired;
        }

        m_txtLevel.text = "Lvl " + GameManager.Instance.MainPlayer.Experience.Level;
    }

    private void UpdateText()
    {
        if (!GameManager.Instance)
            return;

        // Update all texts for now
        for (int i = 0; i < NumInventorySlots; ++i)
        {
            ItemCount ic = GameManager.Instance.MainPlayer.Inventory.GetSlotInformation(i);

            InventorySlotUI isu = m_inventorySlot[i];

            if (ic.Count == 0)
            {
                isu.m_count.text = "";
            }
            else
            {
                isu.m_count.text = ic.Count.ToString();
            }

            isu.m_title.color = (i == m_selectSlotIndex)
            ? new Color(48 / 255f, 255 / 255f, 42 / 255f)
            : new Color(1f, 1f, 1f);

            // set what is owned by this inventory slot
            if (ic.Item == EItem.None)
            {
                isu.m_icon.sprite = null;
                isu.m_icon.color = new Color(1f, 1f, 1f, 0f);
            }
            else
            {
                if (isu.m_icon.sprite == null)
                {
                    TileResourceDef tileResourceDef = TileMapping.GetTileResourceDef((ETile) ic.Item);
                    Texture2D tex = tileResourceDef != null
                        ? Resources.Load(tileResourceDef.Filename) as Texture2D
                        : null;

                    if (tex)
                    {
                        Sprite sprite = Sprite.Create(tex, tileResourceDef.Rect, Vector2.zero);
                        isu.m_icon.sprite = sprite;
                        isu.m_icon.color = ItemInstance.GetColorForItem(ic.Item);
                        isu.m_icon.rectTransform.SetWidth(tileResourceDef.Rect.width);
                        isu.m_icon.rectTransform.SetHeight(tileResourceDef.Rect.height);
                    }
                }
            }
        }
    }

    private void OnClickedInventory(int param)
    {
        SelectedInventorySlot = param;
    }

    private void SetupEvents(InventorySlotUI ui)
    {
        ui.m_button.onClick.RemoveAllListeners();
        ui.m_button.onClick.AddListener(() => OnClickedInventory(ui.m_index));
    }

    private void Duplicate()
    {
        if (m_initialized)
            return;

        if (m_inventorySlot.Count == 0 || m_canvas == null)
            return;

        RectTransform rtOriginal = m_inventorySlot[0].m_parent.GetComponent<RectTransform>();

        for (int i = 1; i < NumInventorySlots; ++i)
        {
            GameObject clone = (GameObject)Instantiate(m_inventorySlot[0].m_parent, m_inventorySlot[0].m_parent.transform.position, m_inventorySlot[0].m_parent.transform.rotation);
            if (clone != null)
            {
                clone.name = "btn_inv (" + i + ")";
                clone.transform.SetParent( m_canvas.transform );
                //clone.transform.parent = m_canvas.transform;
                clone.transform.position = m_inventorySlot[0].m_parent.transform.position;
                clone.transform.localScale = new Vector3(1.0f, 1.0f, 1.0f);

                InventorySlotUI ui = InventorySlotUI.CreateFromGameObject(clone, i);
                m_inventorySlot.Add(ui);

                RectTransform rtClone = clone.GetComponent<RectTransform>();
                Vector2 origin = rtOriginal.localPosition;
                float width = rtOriginal.GetWidth();
                rtClone.localPosition = origin + new Vector2((width + 5) * i, 0f);

                SetupEvents(ui);

                if (i == 9)
                    ui.m_title.text = "0";
                else
                    ui.m_title.text = (i + 1).ToString();

            }
        }

        UpdateText();
        m_initialized = true;
    }

    // Text functions
    public DynamicText DisplayTextWithDuration(string text, Vector2 position, float duration)
    {
        if (!m_txtDynamicPrefab)
            return null;

        GameObject newTxt = (GameObject) Instantiate(m_txtDynamicPrefab, position, Quaternion.identity);
        if (!newTxt)
        {
            Debug.Log("Could not instantiate {0} in DisplayTextWithDuration", m_txtDynamicPrefab);
            return null;
        }

        newTxt.transform.SetParent( m_canvas.transform, true );

        Text textComponent = newTxt.GetComponent<Text>();
        if (!textComponent)
        {
            Debug.Log("Could not get TextComponent of {0} in DisplayTextWithDuration", newTxt);
        }

        textComponent.text = text;

        DynamicText dynamicText = new DynamicText {Object = newTxt, DisplayTime = duration};
        m_dynamicTexts.Add(dynamicText);

        return dynamicText;
    }
}
