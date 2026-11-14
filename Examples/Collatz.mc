# Calculates the number of steps
# for number x to reach 1 by
# performing steps described in
# the Collatz Conjencture

function collatz(x)
{
    if(x == 1)
    {
        return 1;
    }

    if(x % 2 == 0)
    {
        return 1 + collatz(x / 2);
    }
    else
    {
        return 1 + collatz(3 * x + 1);
    }
}

function main()
{
    # Return true if all are true
    x1 = (collatz(32) == 6);
    x2 = (collatz(31) == 107);
    x3 = (collatz(23) != 6);
    x4 = (collatz(5) == 6);
    x5 = (collatz(54) == 113);
    
    return (x1 && x2 && x3 && x4 && x5);
}

main();