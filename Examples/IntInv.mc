# Inverses an integer
# e.g. 123 becomes 321

function irev(x)
{
    y = 0;

    while(x > 0)
    {
        c = x % 10;
        x = x / 10;

        y = 10 * y + c;
    }

    return y;
}

function main()
{
    return irev(13579);
}

main();