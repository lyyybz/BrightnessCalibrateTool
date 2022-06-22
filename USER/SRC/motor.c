#include "motor.h"
#include "device_info.h"
#include "hardware_layer.h"
#include "comfunc.h"

#if CONFIG_DEBUG

/*四相五线电机，每次周期5.625度，旋转一圈需要64个周期
*/
static void _motor_rorate_a_step(int table[6][4])
{

    int i,j;

    for ( i = 0 ; i < 6; i++)
    {
        for (j =0; j < 4; j++)
        {
            set_motor_pin(j,table[i][j]);
        }

        delay_ms(3);
    }

}

void motor_forward_a_step()
{
    int table[][4]= {
        {1,0,0,1},
        {1,1,0,0},
        {0,1,1,0},
        {0,0,1,1},
        {1,0,0,1},
        {0,0,0,0}
    };
    devinfo.steps++;
    _motor_rorate_a_step(table);
}

void motor_backward_a_step()
{
    int table[][4]= {
        {1,1,0,0},
        {1,0,0,1},
        {0,0,1,1},
        {0,1,1,0},
        {1,1,0,0},
        {0,0,0,0}
    };
    _motor_rorate_a_step(table);
    devinfo.steps--;
}

void motor_keep_forward()
{
    int j =0;
    while(1)
    {
        motor_forward_a_step();
        j++;
    }
}

void motor_forward_steps(int steps)
{
    int i=0;
    for(i=0; i< steps; i++)
    {
        motor_forward_a_step();
    }
}


#endif
