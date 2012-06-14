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
 * A QR Code.
 *
 * @see http://code.google.com/apis/chart/docs/gallery/qr_codes.html
 * @since 0.6
 */
class GoogleQRCode extends GoogleChartApi
{
	/**
	 * Width
	 */
	protected $width = '';

	/**
	 * Height
	 */
	protected $height = '';

	protected $choe = false;
	protected $output_encoding = 'UTF-8';
	
	protected $chld = false;
	protected $error_correction_level = 'L';
	protected $margin = 4;

	protected $data = null;

	public function __construct($width, $height)
	{
		$this->width = $width;
		$this->height = $height;
	}

	public function setData($data)
	{
		$this->data = $data;
	}

	/**
	 * @param $output_encoding 
	 * - UTF-8 [Default]
	 * - Shift_JIS
	 * - ISO-8859-1
	 */
	public function setOutputEncoding($output_encoding)
	{
		$valid_encodings = array('UTF-8', 'Shift_JIS', 'ISO-8859-1');
		if ( ! in_array($output_encoding, $valid_encodings) ) {
			throw new InvalidArgumentException('Invalid encoding. Must be one of the following : '.implode(',',$valid_encodings));
		}
		
		$this->choe = true;
		$this->output_encoding = $output_encoding;
		return $this;
	}

	public function setErrorCorrectionLevel($level)
	{
		$valid_levels = array('L','M','Q','H');
		if ( ! in_array($level, $valid_levels) ) {
			throw new InvalidArgumentException('Invalid correction level. Must be one of the following : '.implode(',',$valid_levels));
		}
		
		$this->chld = true;
		$this->error_correction_level = $level;
		return $this;
	}

	public function setMargin($margin)
	{
		if ( !is_numeric($margin) || $margin < 0 ) {
			throw new InvalidArgumentException('Invalid margin (must be > 0)');
		}
		
		$this->chld = true;
		$this->margin = $margin;
		return $this;
	}

	protected function computeQuery()
	{
		$q = array(
			'cht' => 'qr',
			'chs' => $this->width.'x'.$this->height
		);

		$this->compute($q);

		$q = array_merge($q, $this->parameters);

		return $q;
	}

	protected function compute(array & $q)
	{
		$q['chl'] = $this->data;
		
		if ( $this->choe ) {
			$q['choe'] = $this->output_encoding;
		}
		if ( $this->chld ) {
			$q['chld'] = sprintf('%s|%d',$this->error_correction_level, $this->margin);
		}
	}
}
