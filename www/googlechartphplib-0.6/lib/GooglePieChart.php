<?php

/** @file
 * This file is part of Google Chart PHP library.
 *
 * Copyright (c) 2010 Rémi Lanvin <remi@cloudconnected.fr>
 *
 * Licensed under the MIT license.
 *
 * For the full copyright and license information, please view the LICENSE file.
 */
 
require_once 'GoogleChart.php';

/**
 * A Pie Chart.
 *
 * This class is specifically dedicated to Pie Chart. While it is technically
 * possible to create a Pie Chart with the default GoogleChart class, this class
 * takes care of all the Pie Chart specifities for you.
 *
 * @include pie_chart.php
 *
 * @see http://code.google.com/apis/chart/docs/gallery/pie_charts.html
 */
class GooglePieChart extends GoogleChart
{
	protected $_compute_data_label = true;

	protected $rotation = 0;

	protected $c = null;

	public function __construct($type, $width, $height)
	{
		parent::__construct($type, $width, $height);
		$this->setScale(0,100);
	}

	public function setRotation($rotation)
	{
		$this->rotation = $rotation;
	}

	public function setRotationDegree($rotation)
	{
		$this->rotation = deg2rad($rotation);
	}
	
	protected function compute(array & $q)
	{
		if ( $this->rotation ) {
			$q['chp'] = $this->rotation;
		}

		parent::compute($q);
		 // pie chart doesn't support data scaling.
		 // however, i still want to compute a scale for encoding format
		unset($q['chds']);
	}
}
