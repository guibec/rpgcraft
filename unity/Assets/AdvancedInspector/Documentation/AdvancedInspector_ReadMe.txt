=== Advanced Inspector ===
by LightStriker Software

The Inspector, or in other engine the "property grid", is one of the most important tool we have to use. 
It's the window to all our data, and if this tool is lacking, everything we do and how fast we do it, will also suffer from it.

There's lot of Inspector package in the Asset Store, but none that we feel give us the "full package". 
To solve this issue, we've been working on this package for over seven months. One of the first thing we found out is, 
the classes behind the Inspector - SerializedProperty, SerializedObject - were also too limited for our needs, 
surprisingly restrained to a single serialization context. We rewrote them from scratch.

In this package, everything how the Inspector draw and handle data have been reworked from the ground up. 
"Advanced Inspector" is what we believe Unity's Inspector should have been from the start.

Our DLL also comes with a fully commented .xml so our comments shows up in your code editor.
Please read the AdvancedInspector_Manual.pdf for more information about the inner working of each parts.

Video of the AdvancedInspector in action; https://www.youtube.com/watch?v=jvctw4HndKY
Video of the ActionBinding in action; https://www.youtube.com/watch?v=OUFMgCBoCao
Manual online; http://lightstrikersoftware.com/docs/AdvancedInspector_Manual.pdf

The list of features supported in this package is too long for a short readme. Please refer to the manual.


== Bugs, Issues, Support ==

You can contact us directly and we will answer as quickly as possible.

Email : admin@lightstrikersoftware.com
Website : www.lightstrikersoftware.com

You can also contact us on Unity's forum, user "LightStriker".


== FAQ ==

- Why a DLL?

It's easier to distribute, but mostly there's some internal code that are not fully debugged outside the scope of the Advanced Inspector. 
For example, we have a TreeView that is not completed, but for the AI - listing non-nested items - it works.

- Can I get access to the source?

Contact us, maybe we can deal something, but no promises.
The package, as is, should perform any task you want it to perform without the need to the source.
The Advanced Inspector is fully expandable.

- Mac? Linux?

We didn't test this on Linux, but we did on Mac. 

- I found a bug!

Contact us right away! We will fix it ASAP.

- I got idea for other features...

Again, contact us! We will look into adding that feature - if humanly possible - as quickly as we can.

- I don't understand how it works, how to use it or how to implement it!

Contact us, we will gladly help you.


== Version ==

1.0:
- Release

1.1:
- Fixed the Camera preview (the window can be dragged around)
- Fixed the Camera perspective/orthographic drop down with new RestrictedAttribute option.
- Added SkinnedMeshRenderer and Text Mesh as converted editor.
- Added a way to decouple data from description in the RestrictedAttribute. Just pass DescriptorPair instead of the object directly.
- Added a missing constructor in ReadOnlyAttribute.
- Changed the constraint field in the RigidBody because "I prefer the checkboxes".
- Non-editable expandable field - such as object deriving from System.Object - now display info in the format "ToString() [Class Name]". Overriding ToString becomes useful. 
- Fixed a null error in the Restricted > Toolbox.
- SizeAttribute can now flag a collection as not-sortable. Useful for collection which the order comes from the code and should not be changed.
- Added a new Attribute; RuntimeResolve which display a field editor based on the object's type or a type returned by a delegate, instead of the field type. 
It can also be used to restrict a Object field to a specific type. For example, you can make a UnityEngine.Object field only display Material.
- Added a new Attribute; CollectionConstructor which let you create a collection item with the constructor and parameter of your choice.
- Moved the asset into Plugins and Plugins/Editor folder, so other language - like JS - can be supported without modification.
- Copied the Examples asset into JavaScript, also used to test other language support.
- Added a class named "ActionBinding" which is a way to bind an event to a data-driven method invokation. Similar to what Unity will roll out in 4.6 with their new GUI... but better.
Take a look at the video for more information. It's an example of what the AI can do, as no editor were writing for that class.

1.2:
- Fixed an issue with Editor object showing the log message about no reference towards them when saving a scene. It was harmless, but annoying.
- Fixed an issue with internal texture showing the same log message.
- Fixed a index exception while deleting an item in some specific cases. It was harmless, but annoying.
- Fixed the multi-edition on boolean editor.
- Update some internal icon; +, -, open, closed and the drag icon. They should be easier to see in both light and dark.
- Added IPreview interface that gives control over the preview panel of the inspector. New class in the DLL; InspectorPreview.
- Added a new attribute; "Tab". Draws toolbar-like tabs at the top of a script, similar to the SkinnedCloth component.
- Added the standard "on scene" collider editor on the Box, Capsule, and Sphere collider.
- Added a class named TypeUtility. It is used only to retrieve a type in all loaded assembly based on its name.
- Added SkinnedMeshRenderer, TextMesh and Mesh editor.
- Added "very large array" support. Any collection having over 50 items only displays 50, and have arrows at the top and bottom to navigate in them.
- Added UDictionary, a fully Unity-serializable dictionary for Unity 4.5 and later. Does not work on previous Unity version.
- Added RangeInt/RangeFloat, struct used to set a minimum and a maximum. They use Unity's MinMaxSlider.
- The Space attribute can now add space before an item and/or after it. Before default size is 0, after default size is 1. Does not change existing behaviour.

1.3:
- IMPORTANT: All classes related to the Advanced Inspector have been moved to the AdvancedInspector namespace, for consistency with other assemblies (Ex.: System.Serializable) and not to pollute the global scope.
- The AdvancedInspector Attribute now have "InspectDefaultItem" property, which emulate the way Unity display items without the need to add [Inspect] on all your items. You can still inspect item Unity would not display by adding the Inspect Attribute.
- Help Attribute can now be tagged on a class, giving a help box in the header or footer. You can also place multiple instance of the Help attributes, which displays multiple help box.
- Help Attribute now has the "Position" property, which place the help box before or after the targeted member, or in case of a class, in the header or footer.
- Expandable Attribute now has the "Expanded" property, which forces an item to be expanded by default when first viewed.
- Group Attribute now has the "Expandable"  property, which when false, the group is not collapsable and remains open, serving has a dividing box.
- Added a layout "header" and "footer" at the top and bottom of the inspector, before and after all the fields.
- Added an interface; IInspectorRunning, which gives access to the Header and Footer zone in the form of a OnHeaderGUI and OnFooterGUI method.
- InspectorField now support SerializableProperties, but please do not use it, it's most likely full of bug since SerializedProperty is anything but generic.
- CTRL + Click expand all the child of the item expanded. (Ex.: All the subobject in a list)
- Removed the "[class name]" part of the label of a list when that item is the same type as the list itself. (Ex.: List<item>, no need to tell you it's an item)
- The collection type is now only display in Advanced or Debug mode.
- Removed the "shadows" from the following icon; add, delete, folder open, folder close.
- Fixed a null exception thrown when using enums in a Dictionary.
- Added cubemap to the supported type in the Preview.
- Reworked the layout a lot, such as adding 4 pixel to the left of the fields so they don't stick to the window. Now nested item are "boxed-in", which should make multi-level items easier to read.
- Lined up lot of items and subitem that were not properly lined up in specific case in both their label and fields.
- Fixed the Example6 where nested class where not serialized.
- Added methods "GetAttributes" and "GetAttributes<T>" to the Inspector field, returning all instance of a multi-attributes.
- Fixed the aspect ratio in the Camera preview on Scene.
- SkinnedMeshRender "Bones" property is no longer "read only", but it is now an advanced property.
- Added LightEditor to support Unity's Lights.
- Added support for the Gradient type, another of those fancy hidden thing in Unity.
- The cursor is now a "grab hand" when hovering over the drag control.
- Added an EditorWindow example of external inspector.

1.31:
- Added a type-check when displaying an object label to see if the type overload ToString, so it would be called only if a proper implementation exist, and not the base one.
- Inspector Level and Sorting are now saved in EditorPrefs.
- Expansion state are now saved in EditorPrefs. 
- Dragging a label to another label automatically performs a copy-paste of that field.

1.32:
- Fixed undo issue with restricted field.
- Fixed undo issue with struct such as Vector, Quaternion, Bound, Rect, RectOffset.
- Fixed undo issue with string.
- Added some shading to the expandable boxes.
- Added the GameObject/Component picker tool.

1.4:
- Added a new attribute; Background, which color the box of expandable item.
- MaskedEnum attribute is replaced by the Enum attribute, which also controls how an enum is displayed.
- Merge the Size attribute with the CollectionConstructor attribute into the new Collection attribute. 
- Toolbars are no longer using the toolbar style by default. You can do row of buttons this way.
- Toolbars are now drawn on the header, because they collided with the separator.
- Descriptor color now colors field instead of label. It made no sense to color the label, and was often hard to read.
- Descriptor with only a color no longer pass on an empty name to the label.
- Added a missing constructor in AdvancedInspector attribute.
- Expandable attribute now has the InspectDefaultItems similar to the AdvancedInspector attribute.
- Fixed an error raised when encountering a type with multiple overload of ToString.
- Fixed an issue when undoing creation or deletion of an item in a collection was not properly refreshing it.
- Fixed an issue that prevented proper undoing of collection reordering. 
- Added multiple display option to the Enum attribute, see the EnumDisplay enum.
- Added multiple display option to the Collection attribute, see the CollectionDisplay enum.
- Added multiple display option to the Restrict attribute, see the RestrictDisplay enum.
- Added a "Collection Locked" option to the contextual menu, it locks every collection from adding/removing/sorting items.
- Added a Tutorial documentation, which gives steps by steps examples of implementation.
- Fixed an issue that prevented copy pasting AnimationCurve.
- Fixed an issue that custom-made Editor that were not overloading RefreshFields would fail.
- Removed the "EditedTypes" and "EditDerived" from the InspectorEditor, as the CustomEditor attribute values are now used.

1.41:
- Fixed a Null Exception on the CameraEditor on Unity 4.6+.
- Fixed a Array initialization issue introduced with the changes to the Collection attribute.
- Fixed a ComponentMonoBehaviour destruction issue. Even while not referenced, the instance would fail to be destroyed.
- Fixed a collection failing to raise the Erase event on ComponentMonoBehaviour.
- Fixed a issue when a Dictionary would contain ComponentMonoBehaviour and would destroy them even when it shouldn't.
- Fixed a 4 pixels layout issue when multiple nested object would be part of a parent collection.
- Fixed an issue where multiple nested instance of the same type would prevent the child node from being expandable.

1.42:
- Added a new interface; "ICopy" which give an object the power to handle how it should be copied over a targeted field. 
- Added a new interface; "ICopiable" which allows an object to decide if it can be copied or not over a targeted field.
- The Inspector now takes the .NET ICloneable interface into account, allowing it to handle the copying. ICopy take priority over ICloneable.
- Fixed a recursive stack overflow in the copy/paste of a self-referenced object.
- Fixed a stack overflow in prefab comparaison with self-referenced object in editor mode.
- Added a "Take Screenshot" option in the Camera editor. It's available in advanced mode.
- The Collection attribute was missing the IRuntimeAttribute interface declaration.
- ActionBinding now properly sorts out properties, ignoring Getter when a Set is needed and vise versa.
- ActionBinging now flags Binding Parameter that are extra - not declared in the constructor - as being external or static, never internal.
- ActionBinging now control if it can be copied over, and what is copied. Same thing for BindingParameters. See ICopy/ICopiable.
- Fixed an issue when reloading the assembly context where Unity would "hang" for a few second while the Inspector rebuilds the type hierarchy tree.
- Added a missing construction in the Space Attribute.
- Fixed the stack overflow in the ComponentMonoBehaviour... again! What was I drinking?
- Fixed a object array initilization issue when using Collection(0) and a inlined field initialization.
- AnimatorEditor no longer display items twice.
- Added to the AdvancedInspector namespace the following; Toolbox, ModalWindow, WindowResult, IModal.
- The modal window are now draggable. 
- Fixed the expansion of collection in Button mode.

1.43:
- Fixed an expansion bug with groups.
- Fixed an expansion bug with multi-edition of collection and dictionary.
- Fixed an issue that would prevent ExternalEditor list from being sortable.
- Added the compile define ADVANCED_INSPECTOR to detect if the tool is installed or not.
- SHIFT+DRAG on labels of integer or float to change the value.
- DOUBLE+CLICK on labels expends or collapsed the item if it's expandable. 
- DescriptorAttribute is now taken into account when afixed to an enum's value. 
- The CollectionAttribute now has a "Enum Type" properties, which binds a collection to an enum's names. See the documentation for an example.
- Fixed an invalid index when a small array is turned bigger using the CollectionAttribute size property. 
- Added missing attribute targets in a few Attribute so they could be added to structs. 
- New Attribute; MethodAttribute, which gives control on how a method is invoked or displayed. For example, you can replace the botton and draw whatever you want.
- ExternalEditor got more control over how it can be drawn; fixed separator or no space reserved for expander.
- The separator in the ExternalEditor is now uncoupled from the separator in the Inspector. Previously they shared the same settings.
- Transform's global position/orientation are not read only anymore, and accessible in Advanced mode instead of Debug.
- Expandable attribute can now be placed on Interfaces. See AIExample_Interface for an example of an implementation. 
- CollectionDisplay "default" value have been renamed to "List", to better reflect what it does.
- UnityEngine.Component that implement an interface can now be properly drag and dropped in a field of that interface type. See AIExample_Interface.

1.44:
- Made "SelectedTab" property public in InspectorField.
- Fixed an issue when mixing groups with tabs.
- Optimized the lookup of FieldEditor attribute. If you had slow down with this attribute, it should be fine now.
- Now able to inspect static field, properties and methods. Static fields and properties are read only, except when the game is playing.
- InspectorField now has a Static property. 
- Added tooltips and URL to scripting documentation for the Camera, Animator, Light, and many other editors.
- Added editors for Animation, RigidBody2D, SpringJoint2D, Sprite Renderer, and many other classes.
- Note that editors for 2D Physic (Ex.: HingeJoint2D) requires Unity 4.5 or higher.
- InspectorField now supports SerializedProperty arrays. Try to not break that feature.
- Fixed an issue with IDataChanged when the item inspected is at the root.
- Exception thrown on a method/delegate invokation now returns the inner exception, which would make debugging your own method a lot easier.
- AdvancedInspector attribute are now properly inherited in classes hierarchy.
- Enum drop down now properly uses Descriptor for their names.
- Removed the compile define, it didn't work from within a library.
- Fixed an issue that in some case an enum dropdown would become uneditable.
- Nicify enum variables.
- Fixed an issue where class help attribute would fail to invoke their referenced methods properly. 
- Fixed an issue that made enum returns an odd value when they were explicitly flagged another. (Ex.: -315) 
- Fixed Restrict attribute when applied to a collection, so it's used by the child field instead of the parent.
- Fixed an issue where private member of a parent class would become invisible in the derived class.

1.5:
- Now support Unity 5.
- No longer support Unity 4.3, only 4.5+.
- Added the Watch window, which allows you to track inspector values. Right click any value and select "Watch".
- Added the Selected History. Use CTRL+Left Arrow and CTRL+Right Arrow to cycle in your previous selection.
- Added the Runtime Save feature; it allows you to save changed value while playing the game. This feature is in beta and may have issues.
- Redone all the examples, should be way easier to use.
- Added a collection of supported Unity type, like Terrain Collider and Clothes. 
- Due to confusion and overlapping functionnality, the Expandable attribute is now limited to fields and properties and only override the expandability of an item.
- Fixed an issue where sortable list would fail.
- Fixed an issue of recursivity with self-referencing objects.
- Fixed an issue with RuntimeResolved attribute in non-dynamic mode.
- Fixed the draggable icon in Unity Pro.
- Minor tweak to the expandable boxes visual; should look smoother.
- Minor changes in multi-selection.

1.51:
- Added support for nested collection.
- Added support for char, decimal, double, byte, short, long, uint, ushort, ulong, sbyte.
- Note that the above types are not serialized by Unity, so a third party serialization is required.
- IDataChanged and OnValidate are now properly called only when a value change, not only when the GUI is modified.
- Fixed a critical issue in the Animation component.
- Fixed a critical issue with the Material array of the Mesh Renderer component.
- Fixed an issue where some dictionary value type wouldn't be recongnized. 
- Fixed an issue where sometimes a value could not be removed from a dictionary. 
- Fixed an issue where some types might not get expandable.
- Fixed an issue where inspected protected members might show up more than once in derived types.
- Fixed an issue where ComponentMonoBehaviour would be destroyed for no reason. 
- Fixed an issue where Unity's internal list by properties might not properly be set.
- Fixed an issue where bitmaps might get downsized or mipmapped in the inspector.
- Fixed an issue where bitmaps might get darker when Unity is set in linear lighting mode. 
- Added a preference window where some settings can be modified. 
- Added a global "Inspect Default Items" preference in Unity Preferences panel. This allow inspection of class that do not have AdvancedInspector attributes.

1.52:
- IMPORTANT: To avoid conflict with UnityEngine.SpaceAttribute, AdvancedInspector.SpaceAttribute have been renamed to SpacingAttribute.
- Added a new attribute; NoPicker. It removed the picking tools from an object field. Useful when the object should not be modified from the inspector.
- Added a new attribute; Title. It replaces Unity Header attribute but can be placed on properties, methods and can be set at runtime.
- Added support from drag-dropping multiple files, or even a folder of files in a collection.
- Added support for Unity Range, Multiline, TextArea, Header and Space attribute. Note that they still don't work on properties.
- Added "style" choice in the preference panel, you can change how the layout looks like.
- Added support for UnityEvent types.
- "Value Scrolling" and "Copy Paste by Drag" control can now be set in the preference panel.
- The preference panel can now disable specific Advanced Inspector inspector override. 
- Value Scrolling now works on composite fields like Vectors, Rect, Bounds, etc. 
- Added support for ALL PropertyDrawer that draws a specific type. This does not include property drawer handling PropertyAttribute.
- Runtime Attribute can now refer to a property instead of a method. 
- Added the property "MaxDisplayedItems" to the Collection attribute, which trigger the large array display at the size you wish.
- Added the preference "Max Displayed Items" that is similar to the above property, but in the preference panel. 
- Added the preference for mass expand/collapse of children item. Default is "Alt" key while pressing an expansion arrow.
- Transform editor shows localEulerAngles instead of localRotation. This property is more friendly with the rotation. 
- Fixed a recursion copy/paste issue.
- Fixed an throw exception when entering an invalid value in a int/float field.
- Fixed an issue with the animation component... again.
- Fixed an issue where Collection(0) would sometime override the current collection with a 0-sized one.
- Fixed CharacterControllerEditor and ClothRendererEditor which gave error in Unity 5 beta 20. Unity 5 support is still sketchy. 
- Fixed an issue with nested struct. It now works fine, but you should know it's a bad idea performance-wise.  
- Fixed an issue with RigidBodyConstraint display.
- Fixed an issue where the "Inspect Default Item" would sometime catch Unity's type while it shouldn't like GUISkin.
- Fixed a layouting issue where a help is applied to an expandable item.
- Fixed an issue where items could show up as being exandable even if they were not. Most in case of UnityEngine.Object.
- Fixed an issue where reordering a list would fail to flag the object dirty.
- Updated the Animator editor to for the new Update Mode property. 
- Fixed a clamping issue with AnimationCurve.
- Fixed a multi-selection issue with floats.
- Fixed an issue where fields would fail recieve CTRL+C/CTRL+V command.
- Fixed an issue where dynamic read only attribute would always be read only.
- Fixed an issue where some specific property drawer *couch*UnityEvent*cough* would keep their data between inspected object. Seriously...

1.53:
- Added support to drag'n'drop component from Project view. Previously only GameObject would work.
- Added option to invoke contextual menu using a modifier key with a left-click.
- Removed the constructor component from the collection attribute. Now this behaviour has its own attribute; Constructor Attribute.
- Fixed an issue for copying ComponentMonoBehaviour with private field in a derived class.
- Fixed an issue where a ComponentMonoBehaviour should not copy its references.
- Brought back 4.3 support since some people are stuck on it for PS/Xbox compatibility. Note that some feature do not work on 4.3.
- Fixed an issue where IRuntimeAttribute would collide with IListAttribute in passing down in a collection.

1.54:
- Added missing constructors in Collection attribute.
- Asset under source control now shows up as Read Only properly when not checked out.
- External Editor now shows a source control bar at the bottom when an inspected item is under source control.
- Fixed some inspector on the 4.3 version.
- Fixed an issue with struct in Unity type, such as Spring Joint in the Wheel Collider.
- Allow Textures to be dragging in as being a Sprite.
- Multi-drag in array are now ordered by name, to avoid unnecessary sorting.
- Fixed an issue where dragging a single item from the inspector to a collection.

1.55:
- Added an extra description in Group attribute; this shows up on the right of the separator.
- Fixed a null in watched/saved references.
- Fixed a conversion exception while dragging the label of a ranged float.
- Fixed a cast exception while dragging a component on a collection.
- Fixed an issue while nesting lights, animator, animation and a few other component types.
- Fixed the UV1 warning on Unity 5 meshes.
- Fixed Inspect attribute condition property. It was simply not working.
- Fixed Restrict attribute, for some reason it was ignoring null values.
- Allow overriding OnDisable of InspectorEditor.
- Fixed a logic error in dynamic RuntimeResolve attribute.

1.56:
- All IRuntimeAttributes can now target a static delegate using a "Class.NestedClass.Method" path to a static method.
- Fixed an issue where protected member would fail to be inspected in default mode even when serialized.
- Background attribute is now a dynamic attribute. 
- Fixed an issue with displaying AnimationCurves. Due to Unity limitation, it is impossible to change the curve's color.
- Fixed an issue with field using Property Drawer nested inside non-Unity object.
- Fixed an issue that would make Property Drawer fail in Unity 4.3.
- Unity 5.0 no longer supported because of deprecation in EditorWindow definition. Now requires Unity 5.1+
- Fixed the Watch window which would break down in Unity 5.1
- Fixed obsolete values in RigidBody2D and Joint2D for Unity 5.1
- For some reason Unity 5.1 dislike indexed PNG, which made some GUI image show as colored.

1.57:
- Fixed an issue of AnimationCurve not being initialized on creation.
- Fixed an issue where Property Drawer - such as UnityEvent - would fail to draw when nested in a collection.
- Fixed an exception thrown when a base class has no valid derivation while using CreateDerived.
- Found and removed a left-over Debug.LogError in the 5.1 version.
- Fixed an issue where none-Unity object value would not be copied properly when dragged and dropped. 
- Fixed a issue while copying ScriptableObject. It was duplicating the object instead of copying the reference.

1.60 (Unity 5.2+):
- Runtime Help attribute now returns a HelpItem, instead of a HelpAttribute.
- Descriptor attribute now returns a Description object instead of itself.
- Most attributes are now unbinded from the class in the DLL. Instead, interfaces are used, which gives you the ability to create your own attributes.
- Fixed an issue when grouped item would be alone in tabs, making nothing showing up.
- UniTools library have been merged with the AdvancedInspector's one and its namespace renamed. 
- Fixed a copy/paste stack overflow when encountered nested static field of the same type. 
- Fixed some type definition such as HingeJoint and SoftJointLimit.
- Renamed an example material so it doesn't fail compilation on Android.
- Fixed an issue of multiselection of layer masks of different values.
- Removed useless CTRL+E menu item.
- Updated numerous editor towards 5.2 definition.
- Revision A; fixed an issue where tabs were not selectable.

1.61:
[CHANGES]
- Removed the nicifying of dictionary keys when they are string values.
- Added an error while trying to draw a Property Drawer from a non-serializable feature, like a dictionary or a property. 
- Added the ADVANCED_INSPECTOR define back in the DLL, as requested.
[FIXES]
- Fixed that toolbars or header would fail to draw without tabs.
- Fixed that subobject tabs would show up multiple time when combined with Display As Parent.
- Added a catch on FieldEditor drawing so that any exception wouldn't prevent the inspector from drawing its remaining stuff.
- Added "Use Auto Mass" property on RigidBody 2D, added in Unity 5.3x.

1.62:
[CHANGES]
- Collection Attribute may now take a method name. This allow to manually set the name of each elements. (See example 7)
[FIXES]
- Compatible with 5.2 again.
- Cleaned and update the examples.
- Added an enum fix submitted by ldhongen1990.
- Test if a type is generic before trying to instance it; some people forget to flag them as being abstract.
- Fixed an issue where hiding the script type would prevent that object from being flagged dirty upon changes.
- Fixed an initialization issue when a collection is "displayed as parent".

1.63:
[CHANGES]
- The separator's style can now be change to a solid color, and you can also choose the highlighted color in the preference.
[FIXES]
- The Collection Attribute item name override wasn't working when displaying a collection in DropDown or Button mode.
- Help boxes on a ReadOnly item doesn't get grayed out anymore.
- Collection of GameObject no longer create new GameObject when adding indexes. 
- IDataChanged callback wasn't invoked when using the picking tool, since it was outside the redraw scope of the inspector.
- Rewrote the copy/paste logic, it was a mess with lot of very deep and unique issues. 
- Rewrote how the AI gather the object's information. It should be a lot faster now, as it no longer need to test for duplicate items. Contact us if anything isn't showing properly anymore.
- Fixed the cursor picking icon, which for some reason was the wrong one for quite some time.

1.64:
[CHANGES]
- DisplayAsParent now have a property named "Hide Parent". When false, the parent field is displayed as a title.
- Expandable attribute now have a property named "Always Expanded", which force an object to be expanded, and prevent it from being collapsible.
[FIXES]
- MeshRenderer and SkinnedMeshRenderer now properly display reflection probes in range and the probe blending option.
- Updated the TerrainCollider to better follow latest Unity's inspector changes.
- IDataChanged event is now handled on subclass, not only the inspector entry point.
- Preventing a type from being expandable from the AdvancedInspector attribute had been broken for a while. Now, Expandable attribute can be placed on class/struct.
- Fixed a visual lining issue with deep nesting.
- Trap "ExitGUIException" from being thrown by Unity for no reason. Should prevent them from showing up in the console.
- Double-clicking labels was ignoring the mass child expand/collapse feature.
- Expand/Collapse child is now more intuitive as it doesn't expand or collapse the parent, allowing you to see the result.
- Fixed an issue with Copy/Paste where constant field be attempted to be pasted on.

1.65:
[CHANGES]
- New option in the preferences; ability to disable the expansion of references. The exception being ComponentMonoBehaviour.
- New option in the preferences; ability to change the naming pattern of collection. 
- Selected tabs are now persistent. 
[FIXES]
- Clean up all the example to remove most of the [AdvancedInspector] attributes or redundant [Inspect]
- Hide the ComponentMonoBehaviour's owner. It wasn't supposed to be displayed in the inspector.
- Make float written with a "," automatically convert to "."
- Fixed an issue when Inspect Default Items is false, and the inspector would remember some item as being expanded.
- Fixed issue with Collection in DropDown or Button mode while displaying value type.

1.66:
[FIXES]
- Removed Light shadow warning about Unity Pro.
- Removed HDR limitation on camera, it's no longer valid.
- Performance issue fixed on the Selection Tracker while duplicating large selection of GameObject.
- Performance issue when entering/exiting play and reloading context.

1.67:
[CHANGES]
- Added a new attribute; IgnoreBase. Allows deriving from class like Button and ignoring its properties when inspecting.
- The Method attribute now has a property named "UndoMessageOnClick". This attempt to record a undo when pressing the button.
- Added support for attribute-bound PropertyDrawers. Note; since they are Serialization bound, they only works with fields, not Property or Methods.
[FIXES]
- Reworked how IDataChanged works, fixing an issue where the event would not be raised in a Restrict Attribute in Toolbox mode.
- Fixed an issue where the IDataChanged flag would be "eaten" by the wrong class.
- Fixed an issue where tabs would show up in an External Editor where they should not.
- Latest version became increasingly imcompatible with Windows Phone 8.0. Fixed the reflection issue in ActionBinding and ComponentMonoBehaviour.
- Insert, Remove, Move Up, Move Down, Move To Top and Move To Bottom on the contextual menu were broken.
- Exceptions caught by the inspector now returns the proper stack trace in the logged error.
- Cleanup done in the Undos. Some functions were not flagging proper undo stacks.
- Rev A: Fixed assert thrown while clicking button of inspected method.
- Rev B: Fixed a null when drawing a Dictionary's new key field.
- Rev B: Fixed an issue where attribute-bound property drawer would not have the "attribute" and "fieldInfo" value set properly.
- Rev C: Managed to shave off 6 pixels off the labels section.
- Rev C: Fixed an issue that prevented inspecting nested Unity type.
- Rev C: Use the object type instead of the declaring type for ToString overload test.

1.68:
[CHANGES]
- The Method attribute has a new property named "IsCoroutine", which allows you to invoke coroutines in the editor. WaitForSeconds is supported, but not YieldCustom.
- Added preferences control over the default and playmode color of the inspector boxing, since we cannot access the Unity's one.
- FieldAttribute added, an attribute-bound FieldEditor. It is like PropertyDrawer, but with far less limitation. See the documentations and examples.
[FIXES]
- Support Unity 5.4;
- Removed some advanced and debug properties from the Animator inspector; Unity had the brilliant idea of spamming warning everytime they are inspected.
- Update Renderes editor for the new 5.4 features.
- Disable contextual collection control when read only.
- Allows custom EditorField being used with CreateDerived fields.
- Prevent drag'n'dropping ComponentMonoBehaviour on other fields.
- Fixed an issue where some properties without a setter would not appear as read only.
- Internal Change; InspectorField.Editor now returns the associated FieldEditor if any.
- Allow to ping the script from a ComponentMonoBehaviour by clicking on it.

1.69:
[CHANGES]
- CreateDerived attribute now has HideClassName property, which hide the class name on the left of the "+" sign. False by default.
- Don't pass down FieldInfo to PropertyDrawer if it's not a field. Makes some PropertyDrawer works on properties instead of only fields! (See UnityEvent)
- Added Unity's Audio VU draw at the bottom of MonoBehaviour that have the proper OnAudioFilterRead implementation. Warning; lot of reflection involved.
[FIXES]
- Fixed a null exception when applying a Collection attribute on an empty dictionary.
- Fixed that an object was not properly flagged as dirty when adding a file by drag'n'drop to a inner collection.
- Fixed an issue where collection of ComponentMonoBehaviour would not duplicate properly in a copy/paste operation.
- Fixed that multiple version of the same MonoBehaviour on the same GameObject had duplication issues with UnityEvent.
- Allow copy-pasting objects that don't have a public constructor.
- Fixed a major performance issue when inspecting deep nesting (level 4+)
- Major speed boost when selecting very complex object.

1.70:
[CHANGES]
- Added the IInspect interface which allows you to inspect extra object. Similar to a MeshRenderer displaying its materials at the bottom.
- Added a Refresh static method to the AdvancedInspectorAttribute. This can force all AI's editor to be refresh and optionnaly rebuilt.
- Added a icon preview beside object fields. Option to show/hide and size of the icons are available in the right-click menu.
- Added an Always Expanded property in the Collection Attribute to make list and dictionary uncollapsable.
[FIXES]
- Fixed the UV Meter in Unity 5.4+
- Fixed a culture variation exception in saving colors in the preferences. 
- Fixed a null occurring while watching a value.

1.71:
[CHANGES]
- Introducing Menu Attribute, a way to add option menu to a field from an attribute.
- TabAttribute is now a IRuntimeAttribute, which means the tab description can be changed per derived type or as you wish.
- Derive from "AIStateMachineBehaviour" if you want your StateMachineBehaviour to be displayed by Advanced Inspector.
[FIXES]
- Performance improvements; 20% refresh speed boost and -40% memory allocation.
- Property drawers bound by attributes were not being recongnized anymore. Fixed!
- Changed how Apply/Revert works with prefabs, since the data was reverted, but not the link to the prefabs. Some issues still need to be resolved.
- DescriptorAttribute applied directly to Enum can now target a static method delegate.
- Apply "SetDirty" when a Method attribute has a undo message so that Prefabs can be considered modified.
- Namespaced some classes that weren't previously.
- Fixed an issue when a field is UnityEngine.Object and targets is hosting a MonoBehaviour.
- Added an "inversing" condition similar to Inspect Attribute to the ReadOnly Attribute.
- Extracted the Selection Tracker from the DLL so that people can change its shortcut, if wanted.
- Rev A: Mistakingly flagged "AddAttribute" in InspectorField as private.

1.60 and above is only supported on Unity 5.2+
You may require to disable custom Advanced Inspector if you wish to use 1.60 on 5.1x.

Tested on 5.2, 5.3, 5.4 and partially on 5.5 beta