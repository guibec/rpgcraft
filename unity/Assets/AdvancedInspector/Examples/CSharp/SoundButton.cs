using UnityEngine;
using UnityEngine.UI;

namespace AdvancedInspector
{
    [IgnoreBase]
    public class SoundButton : Button
    {
        [Inspect(-10)]
        private bool Interactable
        {
            get { return base.interactable; }
            set { base.interactable = value; }
        }

        [Inspect(-9)]
        private new Transition Transition
        {
            get { return base.transition; }
            set { base.transition = value; }
        }

        [Inspect("IsColorOrSprite")]
        private Graphic TargetGraphic
        {
            get { return base.targetGraphic; }
            set { base.targetGraphic = value; }
        }

        #region Color
        [Inspect("IsColor")]
        private Color NormalColor
        {
            get { return base.colors.normalColor; }
            set
            {
                ColorBlock block = base.colors;
                block.normalColor = value;
                base.colors = block;
            }
        }

        [Inspect("IsColor")]
        private Color HighlightedColor
        {
            get { return base.colors.highlightedColor; }
            set
            {
                ColorBlock block = base.colors;
                block.highlightedColor = value;
                base.colors = block;
            }
        }

        [Inspect("IsColor")]
        private Color PressedColor
        {
            get { return base.colors.pressedColor; }
            set
            {
                ColorBlock block = base.colors;
                block.pressedColor = value;
                base.colors = block;
            }
        }

        [Inspect("IsColor")]
        private Color DisabledColor
        {
            get { return base.colors.disabledColor; }
            set
            {
                ColorBlock block = base.colors;
                block.disabledColor = value;
                base.colors = block;
            }
        }

        [Inspect("IsColor"), RangeValue(1, 5)]
        private float ColorMultiplier
        {
            get { return base.colors.colorMultiplier; }
            set
            {
                ColorBlock block = base.colors;
                block.colorMultiplier = value;
                base.colors = block;
            }
        }

        [Inspect("IsColor")]
        private float FadeDuration
        {
            get { return base.colors.fadeDuration; }
            set
            {
                ColorBlock block = base.colors;
                block.fadeDuration = value;
                base.colors = block;
            }
        }
        #endregion

        #region Sprite
        [Inspect("IsSprite")]
        private Sprite HighlightedSprite
        {
            get { return base.spriteState.highlightedSprite; }
            set
            {
                SpriteState state = base.spriteState;
                state.highlightedSprite = value;
                base.spriteState = state;
            }
        }

        [Inspect("IsSprite")]
        private Sprite PressedSprite
        {
            get { return base.spriteState.pressedSprite; }
            set
            {
                SpriteState state = base.spriteState;
                state.pressedSprite = value;
                base.spriteState = state;
            }
        }

        [Inspect("IsSprite")]
        private Sprite DisabledSprite
        {
            get { return base.spriteState.disabledSprite; }
            set
            {
                SpriteState state = base.spriteState;
                state.disabledSprite = value;
                base.spriteState = state;
            }
        }
        #endregion

        #region Animation
        [Inspect("IsAnimation", -8)]
        private string NormalTrigger
        {
            get { return base.animationTriggers.normalTrigger; }
            set
            {
                AnimationTriggers triggers = base.animationTriggers;
                triggers.normalTrigger = value;
                base.animationTriggers = triggers;
            }
        }

        [Inspect("IsAnimation", -8)]
        private string HighlightedTrigger
        {
            get { return base.animationTriggers.highlightedTrigger; }
            set
            {
                AnimationTriggers triggers = base.animationTriggers;
                triggers.highlightedTrigger = value;
                base.animationTriggers = triggers;
            }
        }

        [Inspect("IsAnimation", -8)]
        private string PressedTrigger
        {
            get { return base.animationTriggers.pressedTrigger; }
            set
            {
                AnimationTriggers triggers = base.animationTriggers;
                triggers.pressedTrigger = value;
                base.animationTriggers = triggers;
            }
        }

        [Inspect("IsAnimation", -8)]
        private string DisabledTrigger
        {
            get { return base.animationTriggers.disabledTrigger; }
            set
            {
                AnimationTriggers triggers = base.animationTriggers;
                triggers.disabledTrigger = value;
                base.animationTriggers = triggers;
            }
        }

        [Inspect("IsAnimation", -7), Spacing(2)]
        private void AutoGenerateAnimation()
        {

        }
        #endregion

        [Inspect]
        private Navigation.Mode Navigation
        {
            get { return base.navigation.mode; }
            set
            {
                UnityEngine.UI.Navigation navigation = base.navigation;
                navigation.mode = value;
                base.navigation = navigation;
            }
        }

        [Inspect(10)]
        private Button.ButtonClickedEvent m_OnClick
        {
            get { return base.onClick; }
            set { base.onClick = value; }
        }

        [SerializeField, Expandable(false)]
        private new AudioClip audio;

        public override void OnPointerClick(UnityEngine.EventSystems.PointerEventData eventData)
        {
            base.OnPointerClick(eventData);

            // Plug your own AudioManager to play the click sound.
            //if (audio != null)
            //    AudioManager.Play(audio);
        }

        private bool IsColor()
        {
            return base.transition == Transition.ColorTint;
        }

        private bool IsColorOrSprite()
        {
            return base.transition == Transition.ColorTint || base.transition == Transition.SpriteSwap;
        }

        private bool IsSprite()
        {
            return base.transition == Transition.SpriteSwap;
        }

        private bool IsAnimation()
        {
            return base.transition == Transition.Animation;
        }
    }
}
