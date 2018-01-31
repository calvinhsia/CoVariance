using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace WpfCoVariance
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            int numIter = 10;

            var lst = new List<MyBase>();
            for (int i = 0; i < numIter; i++)
            {
                MyBase x = null;
                if (i % 2 == 0)
                {
                    x = new MyDerivedA();
                }
                else
                {
                    x = new MyDerivedB();
                }
                lst.Add(x);
            }
            foreach(var instance in lst)
            {
                var result = instance.DoSomething();
                if (instance is MyDerivedA)
                {
                    Debug.Assert(result == $"{nameof(MyDerivedA)}:{nameof(instance.DoSomething)}", "Didn't get right value");
                }
                else
                {
                    Debug.Assert(result == $"{nameof(MyDerivedB)}:{nameof(instance.DoSomething)}", "Didn't get right value");
                }
            }
        }
    }

    abstract class MyBase
    {
        public abstract string DoSomething();
    }
    class MyDerivedA : MyBase
    {
        public override string DoSomething()
        {
            return $"{nameof(MyDerivedA)}:{nameof(DoSomething)}";
        }
    }
    class MyDerivedB : MyBase
    {
        public override string DoSomething()
        {
            return $"{nameof(MyDerivedB)}:{nameof(DoSomething)}";
        }
    }
}
