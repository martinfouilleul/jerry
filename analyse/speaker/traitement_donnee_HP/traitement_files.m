%% Chirp for low amplitude
clear 
close all
clc

for low_amp = 1
    
    load('chirp_amplow.mat') ;
    
    time = chirp_amplow.X.Data ;                % Time Vector
    u1 = chirp_amplow.Y(1).Data ;               % Tension       
    x1 = chirp_amplow.Y(3).Data ;               % Displacement
    te = time(2) - time(1) ;
    fe = 1/te ;                                 % Sample rate
    x1 = x1 - mean(x1) ;                        % Displacement without offset
     
    df = fe/length(time) ; 
    f = df:df:fe ; 

    figure(2)

    subplot 121 
    plot(time,u1)
    xlabel('Time (s)')
    ylabel('Tension (V)')
    title('Input tension')
    legend('Tension')

    subplot 122
    plot(time,x1)
    xlabel('Time (s)')
    ylabel('Displacement (m)')
    title('Output displacement')
    legend('Displacement')

    E = fft(u1); 
    S = fft(x1); 
    H = S./E ;  
    
    figure(3)
    semilogx(f,20*log10((abs(H)))) 
    xlabel('Frequency')
    ylabel('Module (dB)')
    title('Transfer function for low amplitude chirp')
    xlim([5 fe/4])
    legend('High Ampl','Low Ampl')
    hold on
  
    
end


%% Chirp for high amplitude

clear
% close all
clc
for high_amp = 1

    load('chirp_amphigh.mat') ;

    
    time = chirp_amphigh.X.Data ;                % Time Vector
    u1 = chirp_amphigh.Y(1).Data ;               % Tension       
    x1 = chirp_amphigh.Y(3).Data ;               % Displacement
    te = time(2) - time(1) ;
    fe = 1/te ;                                 % Sample rate
    x1 = x1 - mean(x1) ;                        % Displacement without offset
     
    df = fe/length(time) ; 
    f = df:df:fe ; 

    E = fft(u1); 
    S = fft(x1); 
    H = S./E ;
     
    figure(3)
    semilogx(f,20*log10((abs(H)))) 
    xlabel('Frequency (Hz)')
    ylabel('Module (dB)')
    title('Transfer function for different input amplitude')
    legend('High Amplitude','Low Amplitude')
    xlim([5 fe/4])
    hold on
  
end
%%
%save('transfer_function_HP.mat','H')





