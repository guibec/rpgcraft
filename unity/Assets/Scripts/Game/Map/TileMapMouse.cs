using UnityEngine;
using System.Collections;

[RequireComponent(typeof(TileMap))]
public class TileMapMouse : MonoBehaviour 
{
    public GameObject selectionObject;

	// Update is called once per frame
	void Update ()
	{
	    if (selectionObject == null)
	        return;

        bool bHideCube = false;

        if (Input.GetMouseButton(0))
        {
            Ray ray = Camera.main.ScreenPointToRay(Input.mousePosition);
            RaycastHit hitInfo;

            if (GetComponent<Collider>().Raycast(ray, out hitInfo, Mathf.Infinity))
            {
                TileMap tm = GetComponent<TileMap>();
                Vector3 tilePoint = tm.FromWorldToTile(hitInfo.point);

                Debug.Log("Tile (" + tilePoint.x + ", " + tilePoint.y + ")");

                tm.SelectedTile = tilePoint;
                selectionObject.transform.position = new Vector3((int)hitInfo.point.x - 0.5f, (int)hitInfo.point.y + 0.5f);
            }
            else
                bHideCube = true;
        }
        else
            bHideCube = true;

        selectionObject.SetActive(!bHideCube);
	}
}
