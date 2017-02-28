using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class BattleManager : MonoBehaviour {
    private Text Hp_Value;

	void Start () {
        Player p = GameManager.Instance.MainPlayer;

        if (p)
            p.HealthComponent.HealthChanged += HealthComponent_HealthChanged;
    }

    private void OnDestroy() {
        GameManager.Instance.MainPlayer.HealthComponent.HealthChanged -= HealthComponent_HealthChanged;
    }

    private void HealthComponent_HealthChanged(object sender, System.EventArgs e)
    {
        
    }

    // Update is called once per frame
    void Update () {
	    	
	}
}
