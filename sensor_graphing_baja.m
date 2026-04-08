% Do NOT use "clear" here, or it will delete the table you just manually imported!
close all; clc;

%% 1. Link the Workspace Data
% Assign your manually imported table to the 'data' variable used in the script.
% Change 'run_data' if MATLAB named your imported table something else.
data = sensortest1; 

%% 2. Process Time Data
% The Teensy saves time in milliseconds. Let's convert to seconds.
time_sec = data.Time_ms_ / 1000;

% Normalize the time so the graph always starts perfectly at 0 seconds
time_sec = time_sec - time_sec(1);

%% 3. Process Shock Travel Data
right_shock_raw = data.RightShock;
left_shock_raw  = data.LeftShock;

%% 4. Process Wheel Speed Data
% Multiply by 100 to get Pulses per Second (Hz).
rear_speed_hz  = data.RearPulses * 100;
front_speed_hz = data.FrontPulses * 100;

%% 5. Graphing: Shock Travel
figure('Name', 'Suspension Telemetry', 'Color', 'w');

% Create Top Plot for Shocks
subplot(2, 1, 1);
plot(time_sec, right_shock_raw, 'r', 'LineWidth', 1.5);
hold on;
plot(time_sec, left_shock_raw, 'b', 'LineWidth', 1.5);
title('Front Suspension Travel', 'FontSize', 14, 'FontWeight', 'bold');
xlabel('Time (Seconds)', 'FontSize', 12);
ylabel('Raw Position (0-4095)', 'FontSize', 12);
legend('Right Front', 'Left Front', 'Location', 'best');
grid on;

%% 6. Graphing: Wheel Speed
% Create Bottom Plot for Speeds
subplot(2, 1, 2);
plot(time_sec, rear_speed_hz, 'k', 'LineWidth', 1.5);
hold on;
plot(time_sec, front_speed_hz, 'g', 'LineWidth', 1.5);
title('Wheel Speed', 'FontSize', 14, 'FontWeight', 'bold');
xlabel('Time (Seconds)', 'FontSize', 12);
ylabel('Speed (Pulses / Sec)', 'FontSize', 12);
legend('Rear Wheel', 'Front Wheel', 'Location', 'best');
grid on;