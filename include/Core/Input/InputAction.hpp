#pragma once

#include <functional>
#include <vector>

#include <Input/InputActionEvent.hpp>
#include <Input/InputCondition.hpp>
#include <Time/TimeCheck.hpp>
#include <Triggers/TriggerGroup.hpp>
#include <Types/Identifiable.hpp>

namespace obe
{
    namespace Input
    {
        /**
        * \brief Function callback type for KeyboardAction
        */
        using ActionCallback = std::function<void(const InputActionEvent& event)>;

        /**
        * \brief Action triggered by one or more Keyboard key(s)
        * @Bind
        */
        class InputAction : public Types::Identifiable
        {
        private:
            ActionCallback m_callback = [](const InputActionEvent& event){};
            Triggers::TriggerGroup* m_actionTrigger;
            std::vector<std::string> m_contexts;
            std::vector<InputCondition> m_combinations;
            bool m_state = true;
            Time::TimeCheck m_interval;
            Time::TimeCheck m_repeat;
        public:
            /**
            * \brief Creates a new KeyboardAction
            * \param id Id of the KeyboardAction
            */
            explicit InputAction(Triggers::TriggerGroup* triggerPtr, const std::string& id);
            /**
             * \brief Adds an InputCondition to the InputAction
             * \param condition An InputCondition to add to the InputAction
             */
            void addCondition(InputCondition condition);
            /**
             * \brief Clears all the InputCondition of the InputAction
             */
            void clearConditions();
            /**
            * \brief Adds a new Callback
            * \param callback A function that will be called when the Action is triggered
            */
            void connect(ActionCallback callback);
            /**
            * \brief Adds a context to the KeyboardAction
            * \param context New context for the KeyboardAction
            */
            void addContext(const std::string& context);
            /**
            * \brief Get all the contexts the KeyboardAction is in
            * \return A std::vector of std::string containing all the contexts
            */
            std::vector<std::string> getContexts() const;
            /**
            * \brief Sets the delay required between two KeyboardAction triggerings
            * \param delay Delay required between two KeyboardAction triggerings (in ms)
            */
            void setInterval(Time::TimeUnit delay);
            /**
            * \brief Gets the delay required between two KeyboardAction triggerings
            * \return The delay required between two KeyboardAction triggerings (in ms)
            */
            Time::TimeUnit getInterval() const;
            /**
            * \brief Sets the delay between two 'Hold' callbacks activations
            * \param delay Delay required between two 'Hold' callbacks activations
            */
            void setRepeat(Time::TimeUnit delay);
            /**
            * \brief Gets the delay between two 'Hold' callbacks activations
            * \return The delay required between two 'Hold' callbacks activations
            */
            Time::TimeUnit getRepeat() const;
            /**
            * \brief Updates the KeyboardAction
            */
            void update();
            /**
             * \brief Check if the InputAction is enabled
             * \return true if the InputAction is enabled, false otherwise
             */
            bool check() const;
        };
    }
}
