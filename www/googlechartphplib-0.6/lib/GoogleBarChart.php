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
 * A Bar Chart.
 *
 * This class is specifically dedicated to Bar Chart. While it is technically
 * possible to create a Bar Chart with the default GoogleChart class, this class
 * takes care of all the Bar Chart specifities for you.
 *
 * @include bar_chart.php
 *
 * @see http://code.google.com/apis/chart/docs/gallery/bar_charts.html
 */
class GoogleBarChart extends GoogleChart
{
	protected $bar_width = 'a';
	protected $bar_spacing = null;

	public function setBarWidth($width)
	{
		$this->bar_width = $width;
		return $this;
	}
	
	public function setBarSpacing($space)
	{
		$this->bar_spacing = $space;
		return $this;
	}
	
	public function setGroupSpacing($space)
	{

	}
	
	public function computeChbh()
	{
		$str = $this->bar_width;

		if ( $this->bar_spacing ) {
			$str .= ','.$this->bar_spacing;
		}

		return $str;
	}
	
	protected function compute(array & $q)
	{
		$q['chbh'] = $this->computeChbh();

		parent::compute($q);
	}
}
