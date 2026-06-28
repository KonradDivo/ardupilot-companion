--[[
    ArduPilot Payload Safety Interceptor
    Gère la sécurité matérielle en surcouche des ordres de la Nvidia Jetson (C++)
--]]

local SCRIPT_NAME     = "SafetyCore"
local VOLTAGE_CRITIC  = 21.9  -- Failsafe matériel si < 21.9V
local SERVO_CH        = 9

gcs:send_text(6, string.format("[%s] Script actif : Sécurité armement initialisée.", SCRIPT_NAME))

function update()
    -- 1. Sécurité Énergétique Interne
    local voltage = battery:get_voltage(0)
    if voltage and voltage < VOLTAGE_CRITIC then
        gcs:send_text(2, string.format("[%s] CRITICAL BATTERY (%.2fV)! Force RTL.", SCRIPT_NAME, voltage))
        vehicle:set_mode(6) -- Forcer le mode RTL (Return To Launch)
        
        -- Override de sécurité : Refermer le mécanisme de largage immédiatement
        SRV_Channels:set_output_pwm_chan_timeout(SERVO_CH, 1000, 2000)
        return update, 1000
    end

    -- 2. Analyse de l'état de vol
    local current_mode = vehicle:get_mode()
    if current_mode == 6 then -- Si le drone est déjà en RTL suite à un autre incident
        -- On bloque le servo en position fermée par sécurité
        SRV_Channels:set_output_pwm_chan_timeout(SERVO_CH, 1000, 500)
    end

    return update, 200 -- Boucle rapide (5Hz)
end

return update, 1000
