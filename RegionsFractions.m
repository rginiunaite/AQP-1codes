% barplots stacked, fractions of cells in different NEW REGIONS

matrixtoplot = zeros (3,11);

data= 'NEW REGIONS DATA/REGIONSNoboundary.csv';% chemical ablation, D3+D6 control,  speed 0.16 CHEMICAL
a= csvread(data);
matrixtoplot(:,1) = a (:,1);

matrixtoplot(3,1) = 100- matrixtoplot(2,1) - matrixtoplot(1,1);


data= 'NEW REGIONS DATA/REGIONS100boundary.csv';% chemical ablation, D3+D6 control,  speed 0.16 CHEMICAL
a= csvread(data);
matrixtoplot(:,2) = a (:,1);

data= 'NEW REGIONS DATA/REGIONS200boundary.csv';% chemical ablation, D3+D6 control,  speed 0.16 CHEMICAL
a= csvread(data);
matrixtoplot(:,3) = a (:,1);

data= 'NEW REGIONS DATA/REGIONS300boundary.csv';% chemical ablation, D3+D6 control,  speed 0.16 CHEMICAL
a= csvread(data);
matrixtoplot(:,4) = a (:,1);


data= 'NEW REGIONS DATA/REGIONS400boundary.csv';% chemical ablation, D3+D6 control,  speed 0.16 CHEMICAL
a= csvread(data);
matrixtoplot(:,5) = a (:,1);


data= 'NEW REGIONS DATA/REGIONS500boundary.csv';% chemical ablation, D3+D6 control,  speed 0.16 CHEMICAL
a= csvread(data);
matrixtoplot(:,6) = a (:,1);

data= 'NEW REGIONS DATA/REGIONS600boundary.csv';% chemical ablation, D3+D6 control,  speed 0.16 CHEMICAL
a= csvread(data);
matrixtoplot(:,7) = a (:,1);

data= 'NEW REGIONS DATA/REGIONS700boundary.csv';% chemical ablation, D3+D6 control,  speed 0.16 CHEMICAL
a= csvread(data);
matrixtoplot(:,8) = a (:,1);


data= 'NEW REGIONS DATA/REGIONS800boundary.csv';% chemical ablation, D3+D6 control,  speed 0.16 CHEMICAL
a= csvread(data);
matrixtoplot(:,9) = a (:,1);


data= 'NEW REGIONS DATA/REGIONS900boundary.csv';% chemical ablation, D3+D6 control,  speed 0.16 CHEMICAL
a= csvread(data);
matrixtoplot(:,10) = a (:,1);


data= 'NEW REGIONS DATA/REGIONSFullboundary.csv';% chemical ablation, D3+D6 control,  speed 0.16 CHEMICAL
a= csvread(data);
matrixtoplot(:,11) = a (:,1);

matrixtoplot = matrixtoplot/100;
figure
bar(matrixtoplot.','stacked')


legend('R1','R2','R3')

ytickformat('%.1f')
%xticks([1, 2,3,4,5,6,7])
ylim([0,1])
%  xticklabels({'No','100','200','300','400','500','Full'})
%  xlabel('Distance between cell centres')
 ylabel('Fraction of cells in different regions')
 xticks([1, 2,3,4,5,6,7,8,9,10,11])
   xticklabels({'0','100','200','300','400','500','600','700','800','900','1020'})
 % xticklabels({'0','100','200','300','400','500','1020'})
 xlabel(['Length of internal confinement boundary, ',char(181),'m'])
 set(gca,'FontSize',36)
 ax=gca; 
 set(gca,'linewidth',4) 
 box on
