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
 * A Map Chart.
 *
 * This class is specifically dedicated to Map Chart. While it is technically
 * possible to create a Map Chart with the default GoogleChart class, this class
 * takes care of all the Map Chart quirks for you.
 *
 * To create a map chart, you need to add one data serie with addData(). Data
 * serie must contains the country code as keys.
 *
 * @include map_chart.php
 *
 * @see http://code.google.com/apis/chart/docs/gallery/map_charts.html
 */
class GoogleMapChart extends GoogleChart
{
	const MAX_WIDTH = 440;
	const MAX_HEIGHT = 220;

	protected $area = 'world';
	protected $colors = null;

	public function __construct($width, $height)
	{
		if ( $width > self::MAX_WIDTH )
			throw new InvalidArgumentException(sprintf('Max width for Map Chart is %d.', self::MAX_WIDTH));
		if ( $height > self::MAX_HEIGHT )
			throw new InvalidArgumentException(sprintf('Max height for Map Chart is %d.', self::MAX_HEIGHT));
		
		parent::__construct('t', $width, $height);
	}

	/**
	 * Set the zoom area (@c chtm).
	 *
	 * @param $area (enum)
	 * One of the following values
	 * - africa
	 * - asia
	 * - europe
	 * - middle_east
	 * - south_america
	 * - usa
	 * - world
	 *
	 * @see http://code.google.com/apis/chart/docs/gallery/map_charts.html#introduction
	 */
	public function setArea($area)
	{
		$area = strtolower($area);
		if ( ! in_array($area, array('africa', 'asia', 'europe', 'middle_east', 'south_america', 'usa', 'world')) )
			throw new InvalidArgumentException('Invalid zoom area');

		$this->area = $area;
		return $this;
	}

	/**
	 * Set the color map (@c chco).
	 *
	 * To set a custom color map, you need to provide the color used for regions
	 * that do not have data assigned, and a range from the minimal value to
	 * the maximal value, with optional intermediate colors.
	 *
	 * @param $default_color (string)
	 * The color of regions that do not have data assigned.
	 * Set null to leave it to the default value (@c BEBEBE).
	 * 
	 * @param $color_range (array)
	 * An array of at least 2 colors (start of the gradient and stop of the gradient)
	 * Optionally, you can have as many intermediate colors as you like.
	 * Set null to leave it to the default value (@c 0000FF to @c FF0000).
	 *
	 * @see http://code.google.com/apis/chart/docs/gallery/map_charts.html#introduction
	 */
	public function setColors($default_color, array $color_range = null)
	{
		if ( $color_range !== null && ! isset($color_range[1]) )
			throw new Exception('Map Chart color range needs at least two values');
		
		$this->colors = array(
			'default' => $default_color === null ? 'BEBEBE' : $default_color,
			'range' => $color_range === null ? array('FFFFFF','FF0000') : $color_range
		);
		return $this;
	}
	
	public function getColors($compute = true)
	{
		if ( ! $compute )
			return $this->colors;

		if ( $this->colors === null )
			return null;

		return $this->colors['default'].','.implode(',', $this->colors['range']);
	}
	
	protected function compute(array & $q)
	{
		if ( ! isset($this->data[0]) )
			throw new RuntimeException('Map Chart needs one data serie.');

		$v = $this->data[0]->getValues();
		$q['chd'] = 't:'.implode(',',$v);
		// country code must be in upercase
		$q['chld'] = strtoupper(implode('',array_keys($v)));
		$q['chtm'] = $this->area;

		if ( $this->fills ) {
			$q['chf'] = implode('|',$this->fills);
		}
		if ( $this->colors ) {
			$q['chco'] = $this->getColors();
		}
		$this->computeTitle($q);

	}
}

/** @example map_chart_full.php
 * A complete example of how to use the GoogleMapChart class.
 */
