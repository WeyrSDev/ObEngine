#pragma once

#include <memory>
#include <vector>

#include <Input/InputButton.hpp>
#include <Input/InputButtonState.hpp>

namespace obe
{
    namespace Input
    {
        /**
        * \brief A class to monitor and update states of an InputButton
        * @Bind
        */
        class InputButtonMonitor
        {
        private:
            InputButton* m_button = nullptr;
            InputButtonState m_buttonState = InputButtonState::Idle;
            unsigned int m_references = 0;
            friend class InputButtonMonitorPtr;
        public:
            /**
             * \brief Constuctor of InputButtonMonition
             * \param button Pointer to the InputButton to monitor
             */
            InputButtonMonitor(InputButton* button);
            /**
             * \brief Gets a pointer to the monitored InputButton
             * \return A pointer to the monitored InputButton
             */
            InputButton* getButton() const;
            /**
             * \brief Gets the state of the InputButton (InputButtonState)
             * \return The enum of value from InputButtonState corresponding to the state of the monitored InputButton
             */
            InputButtonState getState() const;
            /**
             * \brief Updates the InputButtonMonitor (needed to modify the linked InputButtonState)
             */
            void update();
            /**
             * \brief Number of reference of this InputButtonMonitor (used by InputButtonMonitorPtr to automatically remove unused InputButtonMonitor)
             * \return An unsigned int containing the number of references of this InputButtonMonitor
             */
            unsigned int getReferences() const;
        };

        /**
         * \brief A class to get the states determined by InputButtonMonitor
         * @Bind
         */
        class InputButtonMonitorPtr
        {
        private:
            InputButtonMonitor* m_monitor;
            static unsigned int amount;
            unsigned int m_id = 0;
        public:
            /**
             * \brief Smart pointer for InputButtonMonitor
             * \param monitor Monitor to manage the memory
             */
            InputButtonMonitorPtr(InputButtonMonitor* monitor);
            /**
             * \brief Access the managed InputButtonMonitor using -> operator
             * \return A pointer to the managed InputButtonMonitor
             */
            InputButton* operator->() const;
            /**
             * \brief Access the managed InputButtonMonitor
             * \return A pointer to the managed InputButtonMonitor
             */
            InputButton* getButton() const;
            /**
             * \brief Gets the State of the InputButton monitored by the manager InputButtonMonitor
             * \return The State (InputButtonState) of the InputButton monitored by the manager InputButtonMonitor
             */
            InputButtonState getState() const;
            /**
             * \brief Destructor of InputButtonMonitorPtr
             */
            ~InputButtonMonitorPtr();
        };

        /**
         * \brief Various functions used to manage InputButtonMonitor
         */
        namespace Monitors
        {
            /**
             * \brief Global list of all InputButtonMonitor
             */
            extern std::vector<std::unique_ptr<InputButtonMonitor>> Monitors;
            /**
             * \brief Updates all the InputButtonMonitor
             */
            void UpdateMonitors();
            /**
             * \brief Update the InputButtonMonitor or returns true if it has no more references to them
             * \param element InputButtonMonitor to update
             * \return true if the InputButtonMonitor should be removed, false otherwise
             */
            bool UpdateMonitorsAndRemoveIfNoReferences(const std::unique_ptr<InputButtonMonitor>& element);
            /**
             * \brief Creates a new InputButtonMonitor from an InputButton id
             * \param buttonId Id of the InputButton to monitor
             * \return An InputButtonMonitorPtr managing the underlying InputButtonMonitor
             */
            InputButtonMonitorPtr Monitor(const std::string& buttonId);
            /**
            * \brief Creates a new InputButtonMonitor from an InputButton pointer
            * \param button Pointer to the InputButton to monitor
            * \return An InputButtonMonitorPtr managing the underlying InputButtonMonitor
            */
            InputButtonMonitorPtr Monitor(InputButton* button);
        }
    }
}