#pragma once
#include <tesla.hpp>

namespace tsl {

    namespace elm {

        class BigCategoryHeader : public ListItem {
        public:
            BigCategoryHeader(const std::string &title, bool hasSeparator = false) : ListItem(title), m_hasSeparator(hasSeparator) {}
            virtual ~BigCategoryHeader() {}

            virtual void draw(gfx::Renderer *renderer) override {
                renderer->drawRect(this->getX() - 2, this->getY() + 12 , 5, this->getHeight() - 12, a(tsl::style::color::ColorHeaderBar));
                renderer->drawString(this->m_text.c_str(), false, this->getX() + 13, this->getBottomBound() - 12, 20, a(tsl::style::color::ColorText));

                if (this->m_hasSeparator)
                    renderer->drawRect(this->getX(), this->getBottomBound(), this->getWidth(), 1, a(tsl::style::color::ColorFrame));
            }

            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                this->setBoundaries(this->getX(), this->getY(), this->getWidth(), tsl::style::ListItemDefaultHeight);
            }

            virtual bool onClick(u64 keys) {
                return false;
            }

            virtual Element* requestFocus(Element *oldFocus, FocusDirection direction) override {
                return nullptr;
            }

        private:
            bool m_hasSeparator;
        };

        class SmallListItem : public Element {
        public:
            /**
             * @brief Constructor
             * 
             * @param text Initial description text
             */
            SmallListItem(const std::string& text, const std::string& value = "")
                : Element(), m_text(text), m_value(value) {
            }
            virtual ~SmallListItem() {}

            virtual void draw(gfx::Renderer *renderer) override {
                if (this->m_touched && Element::getInputMode() == InputMode::Touch) {
                    renderer->drawRect(ELEMENT_BOUNDS(this), a(tsl::style::color::ColorClickAnimation));
                }

                if (this->m_maxWidth == 0) {
                    if (this->m_value.length() > 0) {
                        auto [valueWidth, valueHeight] = renderer->drawString(this->m_value.c_str(), false, 0, 0, 15, tsl::style::color::ColorTransparent);
                        this->m_maxWidth = this->getWidth() - valueWidth - 70;
                    } else {
                        this->m_maxWidth = this->getWidth() - 40;
                    }

                    /*
                    size_t written = 0;
                    renderer->drawString(this->m_text.c_str(), false, 0, 0, 15, tsl::style::color::ColorTransparent, this->m_maxWidth, &written);
                    this->m_trunctuated = written < this->m_text.length();

                    if (this->m_trunctuated) {
                        this->m_maxScroll = this->m_text.length() + 8;
                        this->m_scrollText = this->m_text + "        " + this->m_text;
                        this->m_ellipsisText = hlp::limitStringLength(this->m_text, written);
                    }
                    */
                    renderer->drawString(this->m_text.c_str(), false, 0, 0, 15, tsl::style::color::ColorTransparent, this->m_maxWidth);
                }

                renderer->drawRect(this->getX(), this->getY(), this->getWidth(), 1, a(tsl::style::color::ColorFrame));
                renderer->drawRect(this->getX(), this->getBottomBound(), this->getWidth(), 1, a(tsl::style::color::ColorFrame));

                const char *text = m_text.c_str();
                if (this->m_trunctuated) {
                    if (this->m_focused) {
                        if (this->m_scroll) {
                            if ((this->m_scrollAnimationCounter % 20) == 0) {
                                this->m_scrollOffset++;
                                if (this->m_scrollOffset >= this->m_maxScroll) {
                                    this->m_scrollOffset = 0;
                                    this->m_scroll = false;
                                    this->m_scrollAnimationCounter = 0;
                                }
                            }
                            text = this->m_scrollText.c_str() + this->m_scrollOffset;
                        } else {
                            if (this->m_scrollAnimationCounter > 60) {
                                this->m_scroll = true;
                                this->m_scrollAnimationCounter = 0;
                            }
                        }
                        this->m_scrollAnimationCounter++;
                    } else {
                        text = this->m_ellipsisText.c_str();
                    }
                }

                renderer->drawString(text, false, this->getX() + 20, this->getY() + 25, 15, a(tsl::style::color::ColorText), this->m_maxWidth);

                renderer->drawString(this->m_value.c_str(), false, this->getX() + this->m_maxWidth + 45, this->getY() + 25, 15, this->m_faint ? a(tsl::style::color::ColorDescription) : a(tsl::style::color::ColorHighlight));
            }

            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                this->setBoundaries(this->getX(), this->getY(), this->getWidth(), tsl::style::ListItemDefaultHeight / 2);
            }

            virtual bool onClick(u64 keys) override {
                if (keys & HidNpadButton_A)
                    this->triggerClickAnimation();
                else if (keys & (HidNpadButton_AnyUp | HidNpadButton_AnyDown | HidNpadButton_AnyLeft | HidNpadButton_AnyRight))
                    this->m_clickAnimationProgress = 0;

                return Element::onClick(keys);
            }


            virtual bool onTouch(TouchEvent event, s32 currX, s32 currY, s32 prevX, s32 prevY, s32 initialX, s32 initialY) override {
                if (event == TouchEvent::Touch)
                    this->m_touched = currX > this->getLeftBound() && currX < this->getRightBound() && currY > this->getTopBound() && currY < this->getBottomBound();
                
                if (event == TouchEvent::Release && this->m_touched) {
                    this->m_touched = false;

                    if (Element::getInputMode() == InputMode::Touch) {
                        bool handled = this->onClick(HidNpadButton_A);

                        this->m_clickAnimationProgress = 0;
                        return handled;
                    }
                }

                    
                return false;
            }
            

            virtual void setFocused(bool state) override {
                this->m_scroll = false;
                this->m_scrollOffset = 0;
                this->m_scrollAnimationCounter = 0;
                this->m_focused = state;
            }

            virtual Element* requestFocus(Element *oldFocus, FocusDirection direction) override {
                return this;
            }

            /**
             * @brief Sets the left hand description text of the list item
             * 
             * @param text Text
             */
            virtual inline void setText(const std::string& text) {
                this->m_text = text;
                this->m_scrollText = "";
                this->m_ellipsisText = "";
                this->m_maxWidth = 0;
            }

            /**
             * @brief Sets the right hand value text of the list item
             * 
             * @param value Text
             * @param faint Should the text be drawn in a glowing green or a faint gray
             */
            virtual inline void setValue(const std::string& value, bool faint = false) {
                this->m_value = value;
                this->m_faint = faint;
                this->m_maxWidth = 0;
            }

        protected:
            std::string m_text;
            std::string m_value = "";
            std::string m_scrollText = "";
            std::string m_ellipsisText = "";

            bool m_scroll = false;
            bool m_trunctuated = false;
            bool m_faint = false;

            bool m_touched = false;

            u16 m_maxScroll = 0;
            u16 m_scrollOffset = 0;
            u32 m_maxWidth = 0;
            u16 m_scrollAnimationCounter = 0;
        };

        class CustomOverlayFrame : public Element {
        public:
            /**
             * @brief Constructor
             * 
             * @param title Name of the Overlay drawn bolt at the top
             * @param subtitle Subtitle drawn bellow the title e.g version number
             */
            CustomOverlayFrame(const std::string& title, const std::string& subtitle) : Element(), m_title(title), m_subtitle(subtitle) {}
            virtual ~CustomOverlayFrame() {
                if (this->m_contentElement != nullptr)
                    delete this->m_contentElement;
                if (this->m_headerElement != nullptr)
                    delete this->m_headerElement;
            }

            virtual void draw(gfx::Renderer *renderer) override {
                renderer->fillScreen(a(tsl::style::color::ColorFrameBackground));
                renderer->drawRect(tsl::cfg::FramebufferWidth - 1, 0, 1, tsl::cfg::FramebufferHeight, a(0xF222));

                renderer->drawString(this->m_title.c_str(), false, 20, 50, 30, a(tsl::style::color::ColorText));
                renderer->drawString(this->m_subtitle.c_str(), false, 20, 70, 15, a(tsl::style::color::ColorDescription));

                //renderer->drawRect(15, (tsl::cfg::FramebufferHeight - 73) / 3, tsl::cfg::FramebufferWidth - 30, 1, a(tsl::style::color::ColorText));
                renderer->drawRect(15, tsl::cfg::FramebufferHeight - 73, tsl::cfg::FramebufferWidth - 30, 1, a(tsl::style::color::ColorText));

                renderer->drawString("\uE0E1  Back     \uE0E0  OK", false, 30, 693, 23, a(tsl::style::color::ColorText));

                if (this->m_contentElement != nullptr)
                    this->m_contentElement->frame(renderer);
                if (this->m_headerElement != nullptr)
                    this->m_headerElement->frame(renderer);
            }

            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                this->setBoundaries(parentX, parentY, parentWidth, parentHeight);

                if (this->m_headerElement != nullptr) {
                    this->m_headerElement->setBoundaries(15, parentY + 80, parentWidth - 30, ((parentHeight - 73 - 80) / 3));
                    this->m_headerElement->invalidate();
                }
                if (this->m_contentElement != nullptr) {
                    this->m_contentElement->setBoundaries(parentX + 35, parentY + 80 + this->m_headerElement->getHeight(), parentWidth - 85, parentHeight - this->m_headerElement->getHeight() - 73 - 80);
                    this->m_contentElement->invalidate();
                }
            }

            virtual Element* requestFocus(Element *oldFocus, FocusDirection direction) override {
                if (this->m_contentElement != nullptr)
                    return this->m_contentElement->requestFocus(oldFocus, direction);
                else
                    return nullptr;
            }

            virtual bool onTouch(TouchEvent event, s32 currX, s32 currY, s32 prevX, s32 prevY, s32 initialX, s32 initialY) {
                // Discard touches outside bounds
                if (currX < this->m_contentElement->getLeftBound() || currX > this->m_contentElement->getRightBound())
                    return false;
                if (currY < this->m_contentElement->getTopBound() || currY > this->m_contentElement->getBottomBound())
                    return false;

                if (this->m_contentElement != nullptr)
                    return this->m_contentElement->onTouch(event, currX, currY, prevX, prevY, initialX, initialY);
                else return false;
            }

            /**
             * @brief Sets the content of the frame
             * 
             * @param content Element
             */
            virtual void setContent(Element *content) final {
                if (this->m_contentElement != nullptr)
                    delete this->m_contentElement;

                this->m_contentElement = content;

                if (content != nullptr) {
                    this->m_contentElement->setParent(this);
                    this->invalidate();
                }
            }

            virtual void setHeader(Element *header) final {
                if (this->m_headerElement != nullptr)
                    delete this->m_headerElement;

                this->m_headerElement = header;

                if (header != nullptr) {
                    this->m_headerElement->setParent(this);
                    this->invalidate();
                }
            }

            /**
             * @brief Changes the title of the menu
             * 
             * @param title Title to change to
             */
            virtual void setTitle(const std::string &title) final {
                this->m_title = title;
            }

            /**
             * @brief Changes the subtitle of the menu
             * 
             * @param title Subtitle to change to
             */
            virtual void setSubtitle(const std::string &subtitle) final {
                this->m_subtitle = subtitle;
            }

        protected:
            Element *m_contentElement = nullptr;
            Element *m_headerElement = nullptr;
            std::string m_title, m_subtitle;
        };

    }

}