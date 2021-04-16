import matplotlib.pyplot as plt

def voltage(adcValue):
    return (-0.000000000000016 * pow(adcValue,4) + 0.000000000118171 * pow(adcValue,3)- 0.000000301211691 * pow(adcValue,2)+ 0.001109019271794 * adcValue + 0.034143524634089)

adc_values = [i for i in range(4095)]
voltages = [voltage(val) for val in adc_values]

print("in {}".format(adc_values))
print("out {}".format(voltages))

# labels the x axis with Months
plt.xlabel('adc values')

# labels the y axis with Number Potatoes
plt.ylabel('voltages')

# # changes the title of our graph
# plt.title('Number of Potatoes Sold by Month')

# # we want to use months as our x ticks, instead of numbers
# plt.xticks(x_pos, months)

plt.plot(adc_values, voltages, color='b')
plt.show()
