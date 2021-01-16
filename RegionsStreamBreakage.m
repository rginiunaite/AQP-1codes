%% boxplot
sim1 = 'Tunnel M11 REGIONS BREAKAGE/StreamBreakageNobound.csv';
M1 = csvread(sim1);
M1 = M1(:,1);

sim2 = 'Tunnel M11 REGIONS BREAKAGE/StreamBreakage100bound.csv';
M2 = csvread(sim2);
M2 = M2(:,1);

sim3 = 'Tunnel M11 REGIONS BREAKAGE/StreamBreakage200bound.csv';
M3 = csvread(sim3);
M3 = M3(:,1);

sim4 = 'Tunnel M11 REGIONS BREAKAGE/StreamBreakage300bound.csv';
M4 = csvread(sim4);
M4 = M4(:,1);

sim2 = 'Tunnel M11 REGIONS BREAKAGE/StreamBreakage400bound.csv';
M5 = csvread(sim2);
M5 = M5(:,1);

sim2 = 'Tunnel M11 REGIONS BREAKAGE/StreamBreakage500bound.csv';
M6 = csvread(sim2);
M6 = M6(:,1);

sim3 = 'Tunnel M11 REGIONS BREAKAGE/StreamBreakage600bound.csv';
M7 = csvread(sim3);
M7 = M7(:,1);

sim4 = 'Tunnel M11 REGIONS BREAKAGE/StreamBreakage700bound.csv';
M8 = csvread(sim4);
M8 = M8(:,1);

sim2 = 'Tunnel M11 REGIONS BREAKAGE/StreamBreakage800bound.csv';
M9 = csvread(sim2);
M9 = M9(:,1);

sim2 = 'Tunnel M11 REGIONS BREAKAGE/StreamBreakage900bound.csv';
M10 = csvread(sim2);
M10 = M10(:,1);

sim3 = 'Tunnel M11 REGIONS BREAKAGE/StreamBreakageFullbound.csv';
M11 = csvread(sim3);
M11 = M11(:,1);


figure
h = boxplot([M1,M2,M3,M4,M5,M6,M7,M8,M9,M10,M11],'Whisker',1);%,'Labels',{'No','100','200','300','400','500','Full'},'Whisker',5);
set(h,{'linew'},{3})
% 
set(gca,'linew',3)

ylim([0 1])
yticks([0.0, 0.2, 0.4, 0.6, 0.8, 1.0])
yticklabels({'0.0', '0.2', '0.4', '0.6', '0.8', '1.0'})
ylabel('Fraction of follower cells not in chains')
xticks([1, 2,3,4,5,6,7,8,9,10,11])
  xticklabels({'0','100','200','300','400','500','600','700','800','900','1020'})
 xlabel(['Length of internal confinement boundary, ',char(181),'m'])

set(gca,'FontSize',36)
ax = gca;

