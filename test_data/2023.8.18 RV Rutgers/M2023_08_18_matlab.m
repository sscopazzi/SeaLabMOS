%  _    _                      _                          _____ _______ _____  
% | |  | |                    | |                        / ____|__   __|  __ \ 
% | |__| | ___  _ __ ___   ___| |__  _ __ _____      __ | |       | |  | |  | |
% |  __  |/ _ \| '_ ` _ \ / _ \ '_ \| '__/ _ \ \ /\ / / | |       | |  | |  | |
% | |  | | (_) | | | | | |  __/ |_) | | |  __/\ V  V /  | |____   | |  | |__| |
% |_|  |_|\___/|_| |_| |_|\___|_.__/|_|  \___| \_/\_/    \_____|  |_|  |_____/ 
% (ASCII font Big from https://www.coolgenerator.com/ascii-text-generator 

% Homebrew CTD Data Grapher / Visualizer
% Author: Sophie Scopazzi
% Author Website: sophiescopazzi.com
% Date: OCT 2022

% always
clear all
close all

%% INSTRUCTIONS - HOW TO USE THIS CODE
% There are comments throughout code as well
% If you want to contact me about this code, use my website

% 1. Put this script in same folder as datafile.txt from CTD
    % Check name of datafile is the same as in the script
% 2. Ensure sensors are in the correct order in the data from the Arduino
    % script, or none of it will make sense (EC, TDS, sal, etc) in order
% 3. Find correct time values in this plot, record for next step
    % Want data from downward travel, upward water column is disturbed
% 4. Re-comment step 3, put the values recorded into DEPLOY DATA PULLING
% 5. Plots or subplots as desired. Set xlims and ylims as necessary

%% 2. DATA INGEST AND CLEANING 
% datafile from CTD, ensure name is correct, script in same folder as data
data = readtable('DATAFILE.TXT');   % Arduino
data1 = readtable('moo1.txt');      % SeaBird deploy 1
data2 = readtable('moo2.txt');      % SeaBird deploy 2
data3 = readtable('moo3.txt');      % SeaBird deploy 3

%% SEABIRD
sb1_dep    = data1(:,3);
sb1_s      = data1(:,5);
sb1_te     = data1(:,7);
sb1_depth    = table2array(sb1_dep);
sb1_sal      = table2array(sb1_s);
sb1_temp     = table2array(sb1_te);

sb2_dep    = data2(:,3);
sb2_s      = data2(:,5);
sb2_te     = data2(:,7);
sb2_depth    = table2array(sb2_dep);
sb2_sal      = table2array(sb2_s);
sb2_temp     = table2array(sb2_te);

sb3_dep    = data3(:,3);
sb3_s      = data3(:,5);
sb3_te     = data3(:,7);
sb3_depth    = table2array(sb3_dep);
sb3_sal      = table2array(sb3_s);
sb3_temp     = table2array(sb3_te);

%% ARDUINO
% ensure the data is labeled correctly, will depend on the file
% get data from data table, BUT it has nans
    EC_nans       =   data(:,1);  %EC data
    TDS_nans      =   data(:,2);  %TDS
    sal_nans      =   data(:,3);  %salinity
    mbar_nans     =   data(:,4);  %mbar
    temp_nans     =   data(:,5);  %temp in C
    dmeter_nans   =   data(:,6);  %depth in meters

        % take nans out of above data
        EC_table      =   rmmissing(EC_nans);
        TDS_table     =   rmmissing(TDS_nans);
        sal_table     =   rmmissing(sal_nans);
        mbar_table    =   rmmissing(mbar_nans);
        temp_table    =   rmmissing(temp_nans);
        dmeter_table  =   rmmissing(dmeter_nans);

    % convert data from table to array for plotting / manipulation
    EC            =   table2array(EC_table);
    TDS           =   table2array(TDS_table);
    sal_uncal     =   table2array(sal_table);
    sal = sal_uncal + 1.35; %1.35 Calibrate Arduino's sal data to Seabird
    mbar          =   table2array(mbar_table);
    temp_uncal    =   table2array(temp_table);
    temp = temp_uncal + 0.2; %0.2 Calibrate Arduino's temp data to Seabird
    dmeter        =   table2array(dmeter_table);

% convert mbar to feet, incase you want to do that
% depth_feet = mbar.*0.033455256555148;

% convert meters to feet, in case you want to use the Mapping Toolbox
% deploy_feet = distdim(dmeter,'meters','feet'); %(needs Mapping Toolbox)
dfeet = dmeter.*3.28084; % I didn't want to use Mapping Toolbox

%% 3. TO FIND CORRECT START / STOP VALUES IN TIME
    % FIND TIME VALUES IN THIS PLOT than re-comment this out
    % look in this plot to find where depth values start and stop
    % this is what you will put in below, the (93:135), etc
    % uncomment below --- 72-75
    n = length(mbar);
    time = 1:n;
    figure;
    plot(time,mbar);

%% 4. DEPLOY DATA PULLING
    % take out only parts of the data for deployments
    % this is the values found 'FIND DEPTH VALUE PLOT'
    % if you have more or less than four, change it
    
    % ONE
    deploy1_EC       =   EC      (100:143,1);
    deploy1_TDS      =   TDS     (100:143,1);
    deploy1_feet     =   dfeet   (100:143,1);
    deploy1_meter    =   dmeter  (100:143,1);
    deploy1_temp     =   temp    (100:143,1);
    deploy1_sal      =   sal     (100:143,1);

    % TWO 
    deploy2_EC       =   EC      (181:225,1);
    deploy2_TDS      =   TDS     (181:225,1);
    deploy2_feet     =   dfeet   (181:225,1);
    deploy2_meter    =   dmeter  (181:225,1);
    deploy2_temp     =   temp    (181:225,1);
    deploy2_sal      =   sal     (181:225,1);

    % THREE
    deploy3_EC       =   EC      (286:318,1);
    deploy3_TDS      =   TDS     (286:318,1);
    deploy3_feet     =   dfeet   (286:318,1);
    deploy3_meter    =   dmeter  (286:318,1);
    deploy3_temp     =   temp    (286:318,1);
    deploy3_sal      =   sal     (286:318,1);

    % FOUR
    deploy4_EC       =   EC      (382:420,1);
    deploy4_TDS      =   TDS     (382:420,1);
    deploy4_feet     =   dfeet   (382:420,1);
    deploy4_meter    =   dmeter  (382:420,1);
    deploy4_temp     =   temp    (382:420,1);
    deploy4_sal      =   sal     (382:420,1);
    
%% 5. GRAPHS
% however many you did, make that many subplot graphs 
% or you can put them on their own plots (not subplot them)
% these are the same with the numbers changed, 1-4
    figure;
    subplot(1,3,1); % subplot since I want four graphs on one figure
    plot(deploy2_temp,deploy2_meter,'red');
    title('First');
    hold
    plot((sb1_temp(24:180)),(sb1_depth(24:180)),'cyan')
    plot(deploy2_sal,deploy2_meter,'blue');
    plot((sb1_sal(24:180)),(sb1_depth(24:180)),'black') %120
    set(gca,'Ydir','reverse'); % to make zero the top of Y axis
    ylim([0 6]);   % use the depth you deployed
    xlim([22 26.5]) % use the range of your data
    legend('soph-temp','sb-temp','soph-sal','sb-sal')
    ylabel('depth (m)');
    xlabel('°C | PSU (ppt)');

        subplot(1,3,2);
        plot(deploy3_temp,deploy3_meter,'red');
        title('Second');
        hold
        plot((sb2_temp(36:183)),(sb2_depth(36:183)),'cyan') %106
        plot(deploy3_sal,deploy3_meter,'blue');
        plot((sb2_sal(36:183)),(sb2_depth(36:183)),'black')
        set(gca,'Ydir','reverse');
        ylim([0 6]);   % use the depth you deployed
        xlim([21 26.5]) % use the range of your data
        legend('soph-temp','sb-temp','soph-sal','sb-sal')
        ylabel('depth (m)');
        xlabel('°C | PSU (ppt)');

    subplot(1,3,3);
    plot(deploy4_temp,deploy4_meter,'red');
    title('Third');
    hold
    plot((sb3_temp(187:252)),(sb3_depth(187:252)),'cyan')
    plot(deploy4_sal,deploy4_meter,'blue');
    plot((sb3_sal(187:252)),(sb3_depth(187:252)),'black')
    set(gca,'Ydir','reverse');
    ylim([0 6]);   % use the depth you deployed
    xlim([21 26.5]) % use the range of your data
    legend('soph-temp','sb-temp','soph-sal','sb-sal')
    ylabel('depth (m)');
    xlabel('°C | PSU (ppt)');  


%%%%%%%%%%%%%%%%%%%%%%%
% Individual plots for each
%     figure;
%     plot(deploy2_temp,deploy2_meter,'red');
%     title('First');
%     hold
%     plot((sb1_temp(24:180)),(sb1_depth(24:180)),'cyan')
%     plot(deploy2_sal,deploy2_meter,'blue');
%     plot((sb1_sal(24:180)),(sb1_depth(24:180)),'black') %120
%     set(gca,'Ydir','reverse'); % to make zero the top of Y axis
%     ylim([0 6]);   % use the depth you deployed
%     xlim([22 26.5]) % use the range of your data
%     legend('soph-temp','sb-temp','soph-sal','sb-sal')
%     ylabel('depth (m)');
%     xlabel('°C | PSU (ppt)');
% 
%         figure;
%         plot(deploy3_temp,deploy3_meter,'red');
%         title('Second');
%         hold
%         plot((sb2_temp(36:183)),(sb2_depth(36:183)),'cyan') %106
%         plot(deploy3_sal,deploy3_meter,'blue');
%         plot((sb2_sal(36:183)),(sb2_depth(36:183)),'black')
%         set(gca,'Ydir','reverse');
%         ylim([0 6]);   % use the depth you deployed
%         xlim([21 26.5]) % use the range of your data
%         legend('soph-temp','sb-temp','soph-sal','sb-sal')
%         ylabel('depth (m)');
%         xlabel('°C | PSU (ppt)');   
% 
% figure;
% plot(deploy4_temp,deploy4_meter,'red');
% title('Third');
% hold
% plot((sb3_temp(187:252)),(sb3_depth(187:252)),'cyan')
% plot(deploy4_sal,deploy4_meter,'blue');
% plot((sb3_sal(187:252)),(sb3_depth(187:252)),'black')
% set(gca,'Ydir','reverse');
% ylim([0 6]);   % use the depth you deployed
% xlim([21 26.5]) % use the range of your data
% legend('soph-temp','sb-temp','soph-sal','sb-sal')
% ylabel('depth (m)');
% xlabel('°C | PSU (ppt)');  
    