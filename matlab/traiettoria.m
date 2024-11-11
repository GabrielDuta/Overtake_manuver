function [x, y, xF1, yF1, xF2, yF2]= traiettoria(t, xA2, xA1, xA, yA)

% n1 numero auto platoon sorpasso

n1 = 3;

%Lunghezza auto
Lx = 4;

%Distanza interveicolari
dx1 = 2;
dy = 2;
dx2 = 2*n1*Lx;


qx = [xA2-4*dx1 11 1 xA1 11 1 xA+dx2 11 1]'; %Vettore posizione/velocità/accelerazione desiderata Leader asse x
qy = [0 0 0 yA+2*dy 0 0 0 0 0]'; %Vettore posizione/velocità/accelerazione desiderata Leader asse y


%Tempo: t=0 inizio moto, t=6 il Leader si trova nella carreggiata parallela
%rispetto al plotone da superare, t = 12 il Leader compie il sorpasso
t1 = 0;
t2 = 6;
t3 = 12;



%Matrice Nota
T = [t1^8 t1^7 t1^6 t1^5 t1^4 t1^3 t1^2 t1 1;...
    8*t1^7 7*t1^6 6*t1^5 5*t1^4 4*t1^3 3*t1^2 2*t1 1 0;...
    56*t1^6 42*t1^5 30*t1^4 20*t1^3 12*t1^2 6*t1 2 0 0;...
    t2^8 t2^7 t2^6 t2^5 t2^4 t2^3 t2^2 t2 1;...
    8*t2^7 7*t2^6 6*t2^5 5*t2^4 4*t2^3 3*t2^2 2*t2 1 0;...
    56*t2^6 42*t2^5 30*t2^4 20*t2^3 12*t2^2 6*t2 2 0 0;...
    t3^8 t3^7 t3^6 t3^5 t3^4 t3^3 t3^2 t3 1;...
    8*t3^7 7*t3^6 6*t3^5 5*t3^4 4*t3^3 3*t3^2 2*t3 1 0;...
    56*t3^6 42*t3^5 30*t3^4 20*t3^3 12*t3^2 6*t3 2 0 0];


%Parametri incogniti delle traiettorie polinomiali
A = T\qx;
B = T\qy;




%Coordinate xy Leader
x = A(1)*t^8+A(2)*t^7+A(3)*t^6+A(4)*t^5+A(5)*t^4+A(6)*t^3+A(7)*t^2+A(8)*t+A(9);
y = B(1)*t^8+B(2)*t^7+B(3)*t^6+B(4)*t^5+B(5)*t^4+B(6)*t^3+B(7)*t^2+B(8)*t+B(9);

Pd = 2;

PL = sqrt(x^2+y^2);

PF = PL-Pd;
PhiL = atan(y/x);

xF1 = PF*cos(PhiL);
yF1 = PF*sin(PhiL);
Phi1 = atan(yF1/xF1);

xF2 = (PF-2)*cos(Phi1);
yF2 = (PF-2)*sin(Phi1);

%fprintf('x: %f\n', x);
%fprintf('y: %f\n', y);
%fprintf('xF1: %f\n', xF1);
%fprintf('yF1: %f\n', yF1);
%fprintf('xF2: %f\n', xF2);
%fprintf('yF2: %f\n', yF2);


