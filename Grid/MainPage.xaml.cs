using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using Grid.Resources;
using GridDX;

namespace Grid
{
	public partial class MainPage : PhoneApplicationPage
	{
		// Constructor
		Direct3DBackground d3dBackground = null;
		public MainPage()
		{
			InitializeComponent();
			AdMediator_82855A.AdSdkError += AdMediator_Bottom_AdError;
			AdMediator_82855A.AdMediatorFilled += AdMediator_Bottom_AdFilled;
			AdMediator_82855A.AdMediatorError += AdMediator_Bottom_AdMediatorError;
			AdMediator_82855A.AdSdkEvent += AdMediator_Bottom_AdSdkEvent;
			
			// Sample code to localize the ApplicationBar
			//BuildLocalizedApplicationBar();
		}

		private void DrawingSurfaceBackground_Loaded(object sender, RoutedEventArgs e)
        {
			if (d3dBackground == null)
            {
                d3dBackground = new Direct3DBackground();

				d3dBackground.NativeResolution = new Windows.Foundation.Size(
            (float)Math.Floor(Application.Current.Host.Content.ActualWidth * Application.Current.Host.Content.ScaleFactor / 100.0f + 0.5f),
            (float)Math.Floor(Application.Current.Host.Content.ActualHeight * Application.Current.Host.Content.ScaleFactor / 100.0f + 0.5f)
            );
				DrawingSurfaceBackground.SetBackgroundManipulationHandler(d3dBackground);
				DrawingSurfaceBackground.SetBackgroundContentProvider(d3dBackground.CreateContentProvider());
            }
        }

		protected override void OnBackKeyPress(System.ComponentModel.CancelEventArgs e)
		{
			e.Cancel = d3dBackground.OnBackButtonPressed();
		}

		void AdMediator_Bottom_AdSdkEvent(object sender, Microsoft.AdMediator.Core.Events.AdSdkEventArgs e)
		{
			System.Diagnostics.Debug.WriteLine("AdSdk event {0} by {1}", e.EventName, e.Name);}

		void AdMediator_Bottom_AdMediatorError(object sender, Microsoft.AdMediator.Core.Events.AdMediatorFailedEventArgs e)
		{
			System.Diagnostics.Debug.WriteLine("AdMediatorError:" + e.Error + " " + e.ErrorCode );
			// if (e.ErrorCode == AdMediatorErrorCode.NoAdAvailable)
			// AdMediator will not show an ad for this mediation cycle
		}

		void AdMediator_Bottom_AdFilled(object sender, Microsoft.AdMediator.Core.Events.AdSdkEventArgs e)
		{
			System.Diagnostics.Debug.WriteLine("AdFilled:" + e.Name);
		}

		void AdMediator_Bottom_AdError(object sender, Microsoft.AdMediator.Core.Events.AdFailedEventArgs e)
		{
			System.Diagnostics.Debug.WriteLine("AdSdkError by {0} ErrorCode: {1} ErrorDescription: {2} Error: {3}", e.Name, e.ErrorCode, e.ErrorDescription, e.Error);
		}

	}
}