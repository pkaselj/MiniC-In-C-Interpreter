function fibonacci(n)
{
    if( n == 0 || n == 1 || n == 2 )
    {
        return 1;
    }

    return fibonacci(n - 1) + fibonacci(n - 2);
}

function main()
{
    fibonacci(13);
}

main();