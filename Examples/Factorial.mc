function factorial(n)
{
    if(n == 1)
    {
        return 1;
    }
    else
    {
        return n * factorial(n - 1);
    }
}

function main()
{
    if(factorial(5) == 120)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

main();
