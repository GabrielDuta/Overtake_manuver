% Run Simulink model simulation for Traiettoria_copia.slx
function[xA2_r, xA1_r, xA_r, yA_r] = traiettoria(t, xA2, xA1, xA, yA)

    % Model name
    modelName = 'Traiettoria_copia';
    open_system(modelName);
    % Print the input arguments to stdout
    fprintf('Input arguments:\n');
    fprintf('t = %.2f\n', t);
    fprintf('xA2 = %.2f\n', xA2);
    fprintf('xA1 = %.2f\n', xA1);
    fprintf('xA = %.2f\n', xA);
    fprintf('yA = %.2f\n', yA);

    simInput = Simulink.SimulationInput(modelName);

    simInput = setVariable(simInput, 't', t);
    simInput = setVariable(simInput, 'xA2', xA2);
    simInput = setVariable(simInput, 'xA1', xA1);
    simInput = setVariable(simInput, 'xA', xA);
    simInput = setVariable(simInput, 'yA', yA);



    out = sim(simInput);

    fprintf('Output:\n');
    fprintf('class: %s\n', class(out.logsout));
    % Print all the out.logsout ElementNames and their values
    elementNames = out.logsout.getElementNames();
    fprintf('Element Names and Values:\n');
    for i = 1:length(elementNames)
        element = out.logsout{1};
        value = element.Values.Data;
        fprintf('Value class: %s    ', class(value));
        fprintf('%s: %.2f\n', elementNames{i}, value);
    end

    %{
    %xA2_r = Matlab.GetVariable(out, 'xF1');
    xA2_r = out.get('xF1');

    xA1_r = out.get('xF2');
    xA_r = out.get('yF1');
    yA_r = out.get('yF2');
    %}

    close_system(modelName);



end
%{

    % Load the model
    load_system([modelName '.slx']);

    % Set parameters in the model
    setVariable('xA2', xA2);
    setVariable('xA1', xA1);
    setVariable('xA', xA);
    setVariable('yA', yA);

    % Set other constants used in the function
    setVariable('n1', 3);
    setVariable('Lx', 4);
    setVariable('dx1', 2);
    setVariable('dy', 2);
    setVariable('dx2', 2*3*4); % 2*n1*Lx

    % Set simulation stop time (adjust as needed)
    set_param(modelName, 'StopTime', '12'); % Set to t3 from the function

    Simulink.SimulationInput(modelName);

    % Run the simulation
    out = sim(modelName);

    % Close the model
    close_system(modelName);

    % Access simulation results
    % Example: Plot the trajectories
    if ~isempty(out.yout)
        figure;
        plot(out.yout{1}.Values.Data, out.yout{2}.Values.Data, 'b-', ...
            out.yout{3}.Values.Data, out.yout{4}.Values.Data, 'r--', ...
            out.yout{5}.Values.Data, out.yout{6}.Values.Data, 'g:');
        xlabel('X Position');
        ylabel('Y Position');
        legend('Leader', 'Follower 1', 'Follower 2');
        title('Vehicle Trajectories During Overtaking');
    end

    % Display completion message
    disp('Traiettoria Copia simulation completed successfully.');
end
%}