public interface ISave<SaveDataType>
{
    /**
     * Loads from persistent storage
     */
    void Load(SaveDataType saveData);

    /**
     * Returns an object that can be serialized for persistent storage
     */
    SaveDataType Save(); 
}