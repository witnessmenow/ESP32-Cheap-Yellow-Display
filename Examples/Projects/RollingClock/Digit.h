
class Digit
{
private:
    int m_value;
    int m_newValue;
    int m_frame;
    int m_height;
    int m_x;
    int m_y;

public:
    Digit(int value);
    ~Digit();
    int Value();
    int Value(int value);
    int NewValue();
    int NewValue(int newValue);
    int Frame();
    int Frame(int frame);
    int Height();
    int Height(int height);
    void SetXY(int x, int y);
    int X();
    int Y();
};

