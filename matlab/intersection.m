function [accA, accB, accC] = intersection(distA, lengthA, speedA, distB, lengthB, speedB, distC, lengthC, speedC, safetyTime)

    %matrix with dist-lenght values
    platoons = [distA, lengthA, speedA, 'A';
                distB, lengthB, speedB, 'B';
                distC, lengthC, speedC, 'C'];

    % sort by ascending distance to the intersection
    platoons = sortrows(platoons, 1);
    p1 = platoons(1, :);
    p2 = platoons(2, :);
    p3 = platoons(3, :);

    % evaluate platoon #1 time of crossing
    % [space between the last vehicle and the intersection divided by the current speed]
    acc1 = 0;
    toc1 = (p1(1)+p1(2))/p1(3);

    % evaluate acceleration of the second platoon
    tt = toc1 + safetyTime;
    acc2 = 2*(p2(1)-(p2(3)*tt))/(tt*tt);
    
    % avoid positive acceleration
    if acc2 > 0
        acc2 = 0;
    end
    
    % evaluate second platoon time of crossing
    vf = sqrt((p2(3)*p2(3))+(2*acc2*p2(1)));
    vm = (p2(3)+vf)/2;
    toc2 = (p2(1)+p2(2))/vm;

    % evaluate acceleration of the third platoon
    tt = toc2 + safetyTime;
    acc3 = 2*(p3(1)-(p3(3)*tt))/(tt*tt);

    if(acc3 > 0)
        acc3 = 0;
    end


    % assign the accelerations to the right platoons
    accelerations = [acc1, acc2, acc3];

    indexA = platoons(:, 4) == 'A';
    indexB = platoons(:, 4) == 'B';
    indexC = platoons(:, 4) == 'C';

    accA = accelerations(indexA);
    accB = accelerations(indexB);
    accC = accelerations(indexC);
end